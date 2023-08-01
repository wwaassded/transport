#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define nullptr NULL

size_t strlen(const char *s) {
    if (s == nullptr)
        return 0;
    else {
        const char *end = s;
        while (*end)
            ++end;
        return end - s;
    }
}

char *strcpy(char *dst, const char *src) {
    assert(dst && src);
    char *head = dst;
    while (*src) {
        *dst = *src;
        ++dst;
        ++src;
    }
    *dst = '\0';
    return head;
}

char *strncpy(char *dst, const char *src, size_t n) {
    assert(dst && src);
    char *ptr = dst;
    while (n != 0) {
        if ((*ptr++ = *src++) == '\0')
            break;
        --n;
    }
    *ptr++ = '\0';
    return dst;
}

char *strcat(char *dst, const char *src) {
    assert(dst && src);
    char *ptr = dst;
    while (*ptr)
        ++ptr;
    while (*src) {
        *ptr = *src;
        ++ptr;
        ++src;
    }
    *ptr = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    assert(s1 && s2);
    do {
        if (*s1++ != *s2++)
            return *(s1 - 1) - *(s2 - 1);
    } while (*s1 || *s2);
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    assert(s1 && s2);
    do {
        if (*s1++ != *s2++)
            return *(s1 - 1) - *(s2 - 1);
    } while ((*s1 || *s2) && (--n != 0));
    return 0;
}

void *memset(void *s, int c, size_t n) {
    if (s != nullptr) {
        char *ptr = (char *) s;
        while (n != 0) {
            *ptr = (unsigned int) c;
            ++ptr;
            --n;
        }
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
    void *ptr = out;
    while (n-- != 0) {
        *(char *) ptr = *(char *) in;
        ++ptr;
        ++in;
    }
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    assert(s1 && s2);
    while (n) {
        if ((int8_t *) s1 != (int8_t *) s2) return *(int8_t *) s1 - *(int8_t *) s2;
        --n;
    }
    return 0;
}

#endif
