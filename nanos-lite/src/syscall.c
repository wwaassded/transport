#include "syscall.h"


#define SYS_yeild 1

extern void yield();


void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;
    switch (a[0]) {
        case SYS_yeild: {
            sys_yield(c);
            break;
        }
        default:
            printf("TEST::FUCK::it is me !");
            panic("Unhandled syscall ID = %d", a[0]);
    }
}


void sys_yield(Context *c) {
    yield();
    c->GPRx = 0;
}