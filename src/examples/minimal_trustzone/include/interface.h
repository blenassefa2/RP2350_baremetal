#ifndef INTERFACE_H
#define INTERFACE_H

/* Function pointer declaration */
typedef int (*funcptr)(int);

/* Non-secure callable functions */
extern int func1(int x);
extern int func2(funcptr callback, int x);

#endif