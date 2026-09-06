#include <stdbool.h>
#include <stddef.h>
#include <types.h>
#include <stdio.h>
#include <errors.h>

bool is_string_number(char *str)
{
  if (str == NULL)
    return false;

  char c = str[0];

  if (c == '-' || c == '+')
    ++str;

  if (*str == 0)
    return false;

  do
    c = *(str++);
  while (c >= '0' && c <= '9');

  return (c == 0);
}

#define U64TOA_OVERFLOW "[u64toa_ovfw]"

char *u64toa(char *str, size_t str_sz, uint64_t value)
{
  char temp[MIN_U64TOA_SIZE];
  size_t i = 0;

  if (value == 0) {
    if (str_sz >= 2) {
      str[0] = '0';
      str[1] = '\0';

      return str;
    }
    return U64TOA_OVERFLOW;
  }

  while (value > 0) {
    temp[i++] = '0' + (value % 10);
    value /= 10;
  }

  if (i < str_sz) {
    size_t j;
    for (j = 0; j < i; ++j)
      str[j] = temp[i - j - 1];

    str[i] = '\0';

    return str;
  }

  return U64TOA_OVERFLOW;
}

// BEGIN RS485 number types
char *real_u32_prec(char *buf, size_t buf_sz, int *len, uint32_t value, uint32_t prec)
{
  int len_or_error;
  if (prec > 1) {
    prec = value % prec;
    value /= prec;
    len_or_error = snprintf(buf, buf_sz, "%lu.%lu", value, prec);
  } else if (prec)
    len_or_error = snprintf(buf, buf_sz, "%lu.0", value);
  else
    len_or_error = snprintf(buf, buf_sz, "%lu", value);

  if ((size_t)len_or_error >= buf_sz) {
    len_or_error = E_REAL_U32_PREC_STRING_BUFFER_TOO_SHORT;
    goto real_u32_prec_error;
  }

  if (len_or_error < 0) {
    len_or_error = E_REAL_U32_PREC_STRING_FORMAT_ERROR;
real_u32_prec_error:
    error_handler(len_or_error);
    len_or_error = 0;
  }

  buf[len_or_error] = 0;

  if (len)
    *len = len_or_error;

  return buf;
}
// END RS485 number types