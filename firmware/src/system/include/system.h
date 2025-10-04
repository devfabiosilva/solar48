#ifndef SYSTEM_H
 #define SYSTEM_H
#include <stdint.h>
#include <cmsis_gcc.h>
#include <stdbool.h>
#include <time.h>

bool sys_try_lock(volatile bool *, TIMEOUT_MS *, uint32_t, const char *);
bool sys_try_lock_if_gbl_is_false(volatile bool *, volatile bool *, TIMEOUT_MS *, uint32_t, const char *);
void sys_unlock(volatile bool *);

#define IS_ALIGNED_32(p) ((((uintptr_t)(p)) & 0x3) == 0)
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define END_SETUP __enable_irq();
#define DISABLE_SETUP __disable_irq();
#endif

