#ifndef CONSOLE_H
  #define CONSOLE_H

#include <solar48_config.h>

void setdate_cmd(char *);
void getdate_cmd(char *);
void help_cmd(char *);
void timestamp_cmd(char *);
void ping_cmd(char *);
void meminfo_cmd(char *);
void milliseconds_cmd(char *);
void cpuinfo_cmd(char *);
void sensors_cmd(char *);

#ifdef WITH_EPEVER_IP_2000
void readep2000_cmd(char *);
#endif

int read_sensors_process(void *);

#endif

