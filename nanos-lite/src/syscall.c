#include "syscall.h"
#include <common.h>
#include <sys/time.h>

#define SYS_exit 0
#define SYS_yeild 1
#define SYS_open 2
#define SYS_read 3
#define SYS_write 4
#define SYS_close 7
#define SYS_lseek 8
#define SYS_brk 9
#define SYS_gettimeofday 19

extern void yield();
extern void halt(int code);
extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);

void sys_yield(Context *c) {
    yield();
    c->GPRx = 0;
}

void sys_exit(Context *c) {
    halt(c->GPR2);
}

void sys_write(Context *c, int fd, const void *buf, size_t count) {
    c->GPRx = fs_write(fd, buf, count);
}

void sys_brk(Context *c) {
    c->GPRx = 0;
}

void sys_open(Context *c, const char *path, int flags, unsigned int mode) {
    c->GPRx = fs_open(path, flags, mode);
}

void sys_close(Context *c, int fd) {
    c->GPRx = fs_close(fd);
}
void sys_read(Context *c, int fd, void *buf, size_t len) {
    panic("TEST");
    c->GPRx = fs_read(fd, buf, len);
}

void sys_lseek(Context *c, int fd, size_t offset, int whence) {
    c->GPRx = fs_lseek(fd, offset, whence);
}

void sys_gettimeofday(Context *c, struct timeval *tv, struct timezone *tz) {
    c->GPRx = io_read(AM_TIMER_UPTIME).us / 100000;
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;
    switch (a[0]) {
        case SYS_exit: { /* case 0 */
            sys_exit(c);
            break;
        }
        case SYS_yeild: { /* case 1 */
            sys_yield(c);
            break;
        }
        case SYS_open: { /* case 2 */
            sys_open(c, (const char *) a[1], a[2], a[3]);
            break;
        }
        case SYS_read: { /* case 3 */
            sys_read(c, a[1], (void *) a[2], a[3]);
            break;
        }
        case SYS_write: { /* case 4 */
            sys_write(c, a[1], (const void *) a[2], a[3]);
            break;
        }
        case SYS_close: { /* case 7 */
            sys_close(c, a[1]);
            break;
        }
        case SYS_lseek: { /* case 8 */
            sys_lseek(c, a[1], a[2], a[3]);
            break;
        }
        case SYS_brk: { /* case 9 */
            sys_brk(c);
            break;
        }
        case SYS_gettimeofday: { /* case 19 */
            sys_gettimeofday(c, (struct timeval *) a[1], (struct timezone *) a[2]);
            break;
        }
        default:
            printf("TEST::FUCK::it is me !");
            panic("Unhandled syscall ID = %d", a[0]);
    }
}
