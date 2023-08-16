#include "syscall.h"

#define SYS_exit 0
#define SYS_yeild 1
#define SYS_write 4
#define SYS_brk 9

extern void yield();
extern void halt(int code);

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
        case SYS_exit: {
            sys_exit(c);
            break;
        }
        case SYS_write: {
            sys_write(c, a[1], (void *) (uintptr_t) a[2], a[3]);
            break;
        }
        case SYS_brk: {
            sys_brk(c);
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

void sys_exit(Context *c) {
    halt(c->GPR2);
}

void sys_write(Context *c, int fd, void *buf, size_t count) {
    if (fd == 1 || fd == 2) {
        char *buf_char = (char *) buf;
        for (size_t i = 0; i < count; ++i)
            putch(buf_char[i]);
        c->GPRx = 0;
    } else {
        panic("TEST HERE!");
        c->GPRx = -1;
    }
}

void sys_brk(Context *c) {
    c->GPRx = 0;
}