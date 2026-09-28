#include <arm_cmse.h>

typedef int (*funcptr_NS)(int) __attribute__((cmse_nonsecure_call));
typedef int (*funcptr)(int);

__attribute__((cmse_nonsecure_entry))
int func1(int x) { return x + 3; }

__attribute__((cmse_nonsecure_entry))
int func2(funcptr callback, int x) {
    funcptr_NS callback_NS = (funcptr_NS)cmse_nsfptr_create(callback);
    int y = callback_NS(x + 1);
    return y + 2;
}