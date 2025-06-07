#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <usb_io.h>
#include <stdlib.h>
#include <errno.h>
#include <rtc.h>
#include <types.h>

#define ARG_MAX_VEC_SZ (size_t)32 // Max argument list
#define ARGUMENT_BUFFER_MAX_SIZE (size_t)384 // Max buffer size
static char *argc_max_vec[ARG_MAX_VEC_SZ];
static char argument_buffer[ARGUMENT_BUFFER_MAX_SIZE];
#define MAX_VEC_END argc_max_vec[ARG_MAX_VEC_SZ - 1]
#define MAX_BUF_END argument_buffer[ARGUMENT_BUFFER_MAX_SIZE -2]
static uint16_t build_argc(char *argument);

#define COMP_ARGUMENT_COMMAND(cmd) \
  if (strncmp(#cmd, argument_buffer, sizeof(#cmd))) {\
    usb_printf("\nInvalid command. Did you mean \"%s\"?\n", #cmd);\
    return;\
  }

#define CMD_BEGIN_ARG(name) \
void name##_cmd(char *argument) \
{\
  uint16_t argc = build_argc(argument);\
\
  COMP_ARGUMENT_COMMAND(name)

#define CMD_BEGIN_NOARG(name) \
void name##_cmd(char *argument) \
{\
  uint16_t argc = build_argc(argument);\
\
  COMP_ARGUMENT_COMMAND(name) \
\
  if (argc) {\
    usb_printf("\n\nToo many arguments: %d. No arguments expected\n\n", (int)argc);\
    return;\
  }

#define CMD_END }

static void print_date(SOLAR48_DATE *sd, uint32_t *timestamp)
{
  get_solar48_date(sd, timestamp);

  usb_printf("\n\n=== SOLAR48 SYSTEM TIME ===\n\n\tDATE (weekday YEAR/MONTH/DAY): %s %d/%d/%d\n\tTIME hh:mm:ss: %d:%d:%d\n\n",\
    get_day_str1((int)sd->week_day), (int)sd->year, (int)sd->month, (int)sd->day, (int)sd->hour, (int)sd->minute, (int)sd->second);
}

// 0 if success, else error
static int has_longint_error(long int *val, char *this_argument, char *argument_name)
{
  if (!is_string_number(this_argument)) {
    usb_printf("\n\nNot a number: %s = %s\n\n", argument_name, this_argument);
    return -1;
  }

  errno = 0;
  *val = strtol((const char *)this_argument, NULL, 10);

  if (errno) {
    usb_printf("\n\nError: %d. Invalid number %s = %s\n\n", (int)errno, argument_name, this_argument);
    return (int)errno;
  }

  return 0;
}

CMD_BEGIN_ARG(setdate)
  if (argc == 0) {
    usb_printf("\n\n=== TIME SETTINGS ===\n\n\tsetdate:\n\tUSAGE: setdate YEAR MONTH DAY HH mm ss\n\n");
    return;
  }

  if (argc < 3) {
    usb_printf("\n\nMissing parameters. Was expected: setdate yyyy mm dd\n\n");
    return;
  }

  if (argc > 6) {
    usb_printf("\n\nToo many arguments. Was expected: yyyy mm dd [hh] [mm] [ss]\n\n");
    return;
  }

  char
    *arg1, *arg2, *arg3,
    *arg4 = NULL, *arg5 = NULL, *arg6 = NULL;

  switch (argc) {
    case 6:
      arg6 = argc_max_vec[5];
    case 5:
      arg5 = argc_max_vec[4];
    case 4:
      arg4 = argc_max_vec[3];
    default:
      arg3 = argc_max_vec[2];
      arg2 = argc_max_vec[1];
      arg1 = argc_max_vec[0];
  }

  long int yyyy, month, dd, hh, min, ss;

  if (has_longint_error(&yyyy, arg1, "yyyy"))
    return;

  if (has_longint_error(&month, arg2, "mm"))
    return;

  if (has_longint_error(&dd, arg3, "dd"))
    return;

  if (arg4 == NULL)
    hh = 0;
  else if (has_longint_error(&hh, arg4, "hh"))
    return;

  if (arg5 == NULL)
    min = 0;
  else if (has_longint_error(&min, arg5, "mm"))
    return;

  if (arg6 == NULL)
    ss = 0;
  else if (has_longint_error(&ss, arg6, "ss"))
    return;

  if ((yyyy < 0) || (yyyy > 65535) || (month < 0) || (month > 255) | (dd < 0) || (dd > 255) ||
     (hh < 0) || (hh > 255) || (min < 0) || (min > 255) || (ss < 0) || (ss > 255)) {
    usb_printf("\n\nCalendar: Number(s) out of range\n\n");
    return;
  }

  SOLAR48_DATE sd;

  sd.year   = (uint16_t)yyyy;
  sd.month  = (uint8_t)month;
  sd.day    = (uint8_t)dd;
  sd.hour   = (uint8_t)hh;
  sd.minute = (uint8_t)min;
  sd.second = (uint8_t)ss;

  if (set_date(&sd))
    print_date(&sd, NULL);
  else
    usb_printf("\n\nInvalid DATE/TIME\n\n");

CMD_END

CMD_BEGIN_NOARG(help)
  usb_printf(\
    "\n\n=== SOLAR48 USAGE ===\n\n"\
    "getdate [timestamp]                -> reads current Solar48 system time or date from parsed timestamp in seconds\n"\
    "help                               -> shows this help\n"\
    "meminfo                            -> reads Solar48 system memory\n"\
    "ping                               -> test connection between host and Solar48\n"\
    "setdate yyyy mm dd [hh] [mm] [ss]  -> sets Solar48 system data. E.g: 'setdate 2025 01 01 15 20 00'\n"\
    "timestamp                          -> returns current system timestamp in seconds\n"
  );
CMD_END

CMD_BEGIN_ARG(getdate)
  uint32_t timestamp, *tm = NULL;
  long int t;

  SOLAR48_DATE sd;

  if (argc == 1) {
    char *arg1 = argc_max_vec[0];

    if (has_longint_error(&t, arg1, "[timestamp]"))
      return;

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

  print_date(&sd, tm);

CMD_END

CMD_BEGIN_NOARG(timestamp)
  usb_printf("\nTIMESTAMP: %u\n", rtc_get_timestamp());
CMD_END

CMD_BEGIN_NOARG(meminfo)
  usb_print_memory_info();
CMD_END

CMD_BEGIN_NOARG(ping)
  usb_printf("\npong\n");
CMD_END


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

