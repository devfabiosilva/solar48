#include <stdbool.h>
#include <stddef.h>

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

