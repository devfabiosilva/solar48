#ifndef SYSTEM_H
 #define SYSTEM_H
#include <stdint.h>
#include <cmsis_gcc.h>
#include <stdbool.h>
#include <time.h>

bool sys_try_lock(volatile bool *, TIMEOUT_MS *, uint32_t, const char *);
void sys_unlock(volatile bool *);

#define END_SETUP __enable_irq();
#define DISABLE_SETUP __disable_irq();
#endif

