#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <usb_io.h>
#include <stdlib.h>
#include <errno.h>
#include <rtc.h>

#define COMP_ARGUMENT_COMMAND(cmd) \
  if (strncmp(#cmd, argument_buffer, sizeof(#cmd))) {\
    usb_printf("\nInvalid command. Did you mean \"%s\"?\n", #cmd);\
    return;\
  }
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

void cmd_help()
{
  usb_printf(\
    "\n\n=== SOLAR48 USAGE ===\n\n"\
    "getdate [timestamp]                -> reads current Solar48 system time or date from parsed timestamp in seconds\n"\
    "help                               -> shows this help"\
    "meminfo                            -> reads Solar48 system memory\n"\
    "ping                               -> test connection between host and Solar48\n"\
    "setdate yyyy mm dd [hh] [mm] [ss]  -> sets Solar48 system data. E.g: 'setdate 2025 01 01 15 20 00'\n"\
    "timestamp                          -> returns current system timestamp in seconds\n"
  );
}

void getdate_cmd(char *argument)
{
  SOLAR48_DATE sd;

  uint16_t argc = build_argc(argument);
  uint32_t timestamp, *tm = NULL;
  long int t;

  COMP_ARGUMENT_COMMAND(getdate)

  if (argc == 1) {
    errno = 0;
    char *arg1 = argc_max_vec[0];
    t = strtol((const char *)arg1, NULL, 10);

    if (errno) {
      usb_printf("\n\nError: %d. Invalid number %s\n\n", (int)errno, arg1);
      return;
    }

    if (valid_timestamp((int64_t)t)) {
      timestamp = (uint32_t)t;
      tm = &timestamp;
    } else {
      usb_printf("\n\nInvalid timestamp range. Valid values 0 .. 2147483647\n\n");
      return;
    }

  } else if (argc) {
    usb_printf("\n\nToo many arguments: %d. Was expected 1\n\n", (int)argc);
    return;
  }

  get_solar48_date(&sd, tm);

  usb_printf("\n\n=== SOLAR48 SYSTEM TIME ===\n\n\tDATE (weekday YEAR/MONTH/DAY): %s %d/%d/%d\n\tTIME hh:mm:ss: %d:%d:%d\n\n",\
    get_day_str1((int)sd.week_day), (int)sd.year, (int)sd.month, (int)sd.day, (int)sd.hour, (int)sd.minute, (int)sd.second);
}

