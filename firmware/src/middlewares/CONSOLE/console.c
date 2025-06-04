#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <usb_io.h>
#include <rtc.h>

#define ARG_MAX_VEC_SZ (size_t)32 // Max argument list
#define ARGUMENT_BUFFER_MAX_SIZE (size_t)384 // Max buffer size
static char *argc_max_vec[ARG_MAX_VEC_SZ];
static char argument_buffer[ARGUMENT_BUFFER_MAX_SIZE];
#define MAX_VEC_END argc_max_vec[ARG_MAX_VEC_SZ - 1]
#define MAX_BUF_END argument_buffer[ARGUMENT_BUFFER_MAX_SIZE -2]

static uint16_t build_argc(char *argument)
{
  char *p = argument, *r, **argc_vec;
  uint16_t argc = 0;

  argument_buffer[0] = 0;

  while ((*p == ' ') && (*p != 0))
    ++p;

  if (*p == 0)
     return 0;

  strncpy(argument_buffer, p, ARGUMENT_BUFFER_MAX_SIZE);
  memset((void *)argc_max_vec, 0, ARG_MAX_VEC_SZ*sizeof(char *));
  argument_buffer[ARGUMENT_BUFFER_MAX_SIZE -1] = 0;

  p = argument_buffer;
  while ((*p != 0) && (*p != ' '))
    p++;

  if (*p == 0)
    return 0;

  *p = 0;

  while (*(++p) == ' ');

  r = p;

  if ((++r) >= &MAX_BUF_END)
    return 0;

  // Special case ex.: "command a" where arg is 1 and has one char
  if (*r == 0) {
    argc_max_vec[0] = p;
    return 1;
  }

  argc_vec = argc_max_vec;

  while (*r != 0) {
    while ((*r != ' ') && (*r != 0)) {
      if (++r >= &MAX_BUF_END)
        break;
    }

    *argc_vec = p;
    *r = 0;

    while (*(++r) == ' ');

    if (r >= &MAX_BUF_END)
      return argc;

    p = r;
    ++argc;

    if ((++argc_vec) > &MAX_VEC_END)
      break;
  }

  return argc;
}

void cmd_set_date(char *argument)
{
  uint16_t argc = build_argc(argument);

  if (argc == 0) {
    usb_printf("\n\n=== TIME SETTINGS ===\n\n\tsetdate:\n\tUSAGE: setdate YEAR MONTH DAY HH mm ss\n\n");
    return;
  }

  usb_printf("\n\nTODO: Not implemented yet\n\n");
}

void cmd_get_date()
{
  static SOLAR48_DATE sd;
  //uint32_t tm = rtc_get_timestamp();
  get_solar48_date(&sd, NULL);
//TODO calculate timestamp if arg 1 is provided
  usb_printf("\n\n=== SOLAR48 SYSTEM TIME ===\n\n\tDATE (weekday YEAR/MONTH/DAY): %s %d/%d/%d\n\tTIME hh:mm:ss: %d:%d:%d\n\n",\
    get_day_str1((int)sd.week_day), (int)sd.year, (int)sd.month, (int)sd.day, (int)sd.hour, (int)sd.minute, (int)sd.second);
}

