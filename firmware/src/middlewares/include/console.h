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
void readep2000_status_cmd(char *);
void readep2000_ovr_temp_cmd(char *);
void ctf_ep2000_cmd(char *);
#endif

void error_cmd(char *);

#ifdef EPEVER_TRACER6415AN
void rd_tr6415_rated_datum_cmd(char *);
void rd_tr6415_real_time_cmd(char *);
#endif

int read_sensors_process(void *);

#endif

