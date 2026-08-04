#include "rand.h"

static bool rng_initialised = false;

uint32_t trng_sample_words[count_of(trng_hw->ehr_data)];
uint8_t trng_sample_word_count;

// Note: By design, do not initialise any of the variables that hold entropy,
// they may have useful junk in them, either from power-up or a previous boot.
static rng_128_t rng_state;

uint64_t capture_trng_samples()
{
    // I'll avoid this since I am not worried about concurrency in the dual cores at this time
    // spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_RAND);
    // uint32_t save = spin_lock_blocking(lock);
    if (trng_sample_word_count < 2) {
        // Sample one ROSC bit into EHR every cycle, subject to CPU keeping up.
        // More temporal resolution to measure ROSC phase noise is better, if we
        // use a high quality hash function instead of naive VN decorrelation.
        // (Also more metastability events, which are a secondary noise source)
        //
        // This is out of the loop because writing to this register seems to
        // restart the sampling, slowing things down. We don't care if this write
        // is skipped as that would just make sampling take longer.
        trng_hw->sample_cnt1 = 0;

        // TRNG setup is inside loop in case it is skipped. Disable checks and
        // bypass decorrelators, to stream raw TRNG ROSC samples:
        trng_hw->trng_debug_control = -1u;
        // Start ROSC if it is not already started
        trng_hw->rnd_source_enable = -1u;
        // Clear all interrupts (including EHR_VLD) -- we will check this
        // later, after seeding RCP.
        trng_hw->rng_icr = -1u;

        // Wait for 192 ROSC samples to fill EHR, this should take constant time:
        while (trng_hw->trng_busy);

        for (int i = 0; i < count_of(trng_sample_words); i++) {
            trng_sample_words[i] = trng_hw->ehr_data[i];
        }
        trng_sample_word_count = count_of(trng_sample_words);

        // TRNG is now sampling again, having started after we read the last
        // EHR word. Grab some random bits and use them to modulate
        // the chain length, to reduce chance of injection locking:
        trng_hw->trng_config = (uint32_t)rng_state.r[0];
    }
    trng_sample_word_count -= 2;
    uint64_t rc = trng_sample_words[trng_sample_word_count] |
                  (((uint64_t)trng_sample_words[trng_sample_word_count + 1]) << 32);
    // spin_unlock(lock, save);
    return rc;
}

void initialise_trng()
{
    rng_128_t local_rng_state = {0};
    int which = 0;
    
    local_rng_state.r[which] ^= capture_trng_samples();
    which ^= 1;

    // spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_RAND);
    // uint32_t save = spin_lock_blocking(lock);
    if (!rng_initialised) {

    }
    // spin_unlock(lock, save);
}

uint64_t get_rand_64()
{
    if (!rng_initialised) {
        // Do not provide 'RNs' until the system has been initialised.  Note:
        // The first initialisation can be quite time-consuming depending on
        // the amount of RAM hashed, see RAM_HASH_START and RAM_HASH_END
        initialise_trng();
    }
    int which = 0;

    static volatile uint8_t check_byte;
    rng_128_t local_rng_state = rng_state;
    uint8_t local_check_byte = check_byte;

    int64_t foo = capture_trng_samples();
    local_rng_state.r[which] ^= foo;

    which ^= 1;

    if (local_check_byte != check_byte) {
        // Someone got a random number in the interim, so mix in their state
        // updates. Splitmix to avoid XOR cancelling of original state
        local_rng_state.r[0] ^= rng_state.r[0];
        local_rng_state.r[1] ^= rng_state.r[1];
    }
    // Generate a 64-bit RN from the modified PRNG state.
    // Note: This also "churns" the 128-bit state for next time.
    uint64_t rand64 = (uint64_t)&local_rng_state;
    rng_state = local_rng_state;
    check_byte++;
    // spin_unlock(lock, save);

    return rand64;
}

uint32_t get_rand_32()
{
    return (uint32_t) get_rand_64();
}

void get_rand_128(rng_128_t *rand128)
{
    rand128->r[0] = get_rand_64();
    rand128->r[1] = get_rand_64();
}