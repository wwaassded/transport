#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define nullptr NULL
int printf(const char *fmt, ...)
{
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...)
{
  assert(out != nullptr);
  char *ptr = out;
  va_list ap;
  va_start(ap, fmt);
  while (*fmt)
  {

    if(*fmt == '%') {
      ++fmt;

      switch(*fmt) {

        case 's': {
          const char *tmp = va_arg(ap,const char *);
          while(*tmp)
            *ptr++ = *tmp++;
          break;
        }
        
      }

    }
    ++fmt;
  }
  *ptr = '\0';
  return ptr - out - 1;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

#endif
