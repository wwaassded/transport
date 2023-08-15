#include "syscall.h"
#include <common.h>


void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = 1;
    printf("7::%d\n", c->gpr[11]);
    switch (a[0]) {
        default:
            printf("TEST::FUCK::it is me !");
            panic("Unhandled syscall ID = %d", a[0]);
    }
}