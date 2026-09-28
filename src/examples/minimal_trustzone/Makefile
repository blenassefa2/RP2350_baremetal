CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
NM      = $(CROSS)nm

CPU_S  = -mcpu=cortex-m33 -mthumb -mfloat-abi=soft
CPU_NS = -mcpu=cortex-m33 -mthumb -march=armv8-m.main+dsp -mfloat-abi=soft
COMMON = -Os -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Wall -Wextra

S_SRCS  = secure/0boot.s secure/main.c secure/interface.c
NS_SRCS = nonsecure/0boot.s nonsecure/main.c

S_ELF  = build/secure.elf
NS_ELF = build/nonsecure.elf
NS_BIN = build/nonsecure.bin
S_IMPLIB = build/secure_cmse.o

buildS:
	mkdir -p build
	$(CC) $(CPU_S) $(COMMON) -mcmse -Tsecure/link.ld \
	      -Wl,--cmse-implib,--out-implib=$(S_IMPLIB) \
	      $(S_SRCS) -lgcc -o $(S_ELF)
	$(OBJCOPY) -O binary $(S_ELF) build/secure.bin
	$(NM) $(S_ELF) | grep -E "func1|func2"

buildNS: buildS
	$(CC) $(CPU_NS) $(COMMON) -Tnonsecure/link.ld \
	      $(NS_SRCS) $(S_IMPLIB) -o $(NS_ELF)
	$(OBJCOPY) -O binary $(NS_ELF) $(NS_BIN)

flash_S_and_NS: buildS buildNS
	openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
	  -c "init" -c "reset halt" \
	  -c "mww 0x20000000 0 0x100" \
	  -c "mww 0x2007C000 0 0x40" \
	  -c "program $(S_ELF) verify" \
	  -c "program $(NS_BIN) 0x10100000 verify" \
	  -c "reset run" -c "sleep 4000" -c "halt" \
	  -c "mdw 0x20000000 4" \
	  -c "mdw 0x2007C000 4" \
	  -c "shutdown"

clear:
	rm -rf build