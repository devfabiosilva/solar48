#include <stdarg.h>
#include <stdio.h>

int strbuf_fmt(char *buf, size_t buf_sz, const char *fmt, ...)
{
  int len;
  va_list arg;

  va_start(arg, fmt);
  len = vsnprintf(buf, buf_sz, fmt, arg);
  va_end(arg);

  if (len >= buf_sz) {
    len = buf_sz - 1;
    buf[len] = 0;
  }

  return len;
}

