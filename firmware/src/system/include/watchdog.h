#ifndef WATCHDOG_H
 #define WATCHDOG_H

#include <stdbool.h>

void init_idw();
void iwd_refresh();
bool iwd_fault();
void reset_wdg_fault();

#endif

