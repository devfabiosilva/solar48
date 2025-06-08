#ifndef CONSOLE_H
  #define CONSOLE_H

#define COMMAND_CHECK_CALL_ARG(command) \
  if (strncmp(text, #command, sizeof(#command)-1) == 0) {\
    command##_cmd(text); \
    return; \
  }
/*
#define COMMAND_CHECK(cmd, ...) \
  if (strncmp(text, cmd, sizeof(cmd)) == 0) { \
    usb_printf(__VA_ARGS__); \
    return; \
  }

#define COMMAND_CHECK_CALL(cmd, fn) \
  if (strncmp(text, cmd, sizeof(cmd)) == 0) {\
    fn(); \
    return; \
  }
#define COMMAND_CHECK(cmd, ...) \
  if (strncmp(text, cmd, sizeof(cmd)) == 0) { \
    usb_printf(__VA_ARGS__); \
    return; \
  }
*/
void setdate_cmd(char *);
void getdate_cmd(char *);
void help_cmd(char *);
void timestamp_cmd(char *);
void ping_cmd(char *);
void meminfo_cmd(char *);
void milliseconds_cmd(char *);

#endif

