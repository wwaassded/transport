#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define nullptr NULL
static char buf[64];
int printf(const char *fmt, ...) {
    uint32_t res = 0;
    va_list ap;
    va_start(ap, fmt);
    while (*fmt != '\n') {
        if (*fmt == '%') {
            fmt = fmt + 1;
            switch (*fmt) {
                case 'c': {
                    char ch = va_arg(ap, int);
                    putch(ch);
                    ++res;
                }
                case 's': {
                    const char *str = va_arg(ap, const char *);
                    putstr(str);
                    res += strlen(str);
                    break;
                }
                case 'd': {
                    uint8_t i;
                    int num = va_arg(ap, int);
                    if (num < 0) {
                        putch('-');
                        num = -num;
                    }
                    for (i = 0; num != 0; ++i) {
                        buf[i] = (num % 10) + '0';
                        num /= 10;
                    }
                    res += i + 1;
                    --i;
                    while (i != 0) {
                        putch(buf[i]);
                        --i;
                    }
                    putch(buf[i]);
                    break;
                }
                case 'u': {
                    uint64_t num = va_arg(ap, uint64_t);
                    uint16_t i;
                    for (i = 0; num != 0; ++i) {
                        buf[i] = (num % 10) + '0';
                        num /= 10;
                        putch(buf[i]);
                    }
                    res += i + 1;
                    --i;
                    while (i != 0) {
                        putch(buf[i]);
                        --i;
                    }
                    putch(buf[i]);
                    break;
                }
                default: {
                    panic("Not implemented");
                }
            }
        } else {
            ++res;
            putch(*fmt);
        }
        fmt = fmt + 1;
    }
    va_end(ap);
    fmt = fmt + 1;
    return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    assert(out != nullptr);
    char *ptr = out;
    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {

        if (*fmt == '%') {
            ++fmt;

            switch (*fmt) {

                case 's': {
                    const char *tmp = va_arg(ap, const char *);
                    while (*tmp)
                        *ptr++ = *tmp++;
                    break;
                }

                case 'd': {
                    int tmp = va_arg(ap, int);
                    uint8_t i;
                    if (tmp < 0) {
                        *ptr++ = '-';
                        tmp = -tmp;
                    }
                    for (i = 0; tmp != 0; ++i) {
                        buf[i] = (tmp % 10) + '0';
                        tmp /= 10;
                    }
                    --i;
                    while (i != 0) {
                        *ptr++ = buf[i];
                        --i;
                    }
                    *ptr++ = buf[i];
                    break;
                }
            }
        } else
            *ptr++ = *fmt;
        ++fmt;
    }
    *ptr = '\0';
    return ptr - out - 1;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
