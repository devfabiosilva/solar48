#include <stdbool.h>
#include <stddef.h>
#include <types.h>

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


