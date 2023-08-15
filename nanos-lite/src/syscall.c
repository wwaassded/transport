#include "syscall.h"
#include <common.h>


void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;

    switch (a[0]) {
        default:
            printf("TEST::FUCK::it is me !");
            panic("Unhandled syscall ID = %d", a[0]);
    }
}
