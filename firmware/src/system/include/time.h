#ifndef TIME_H
 #define TIME_H

#include <stdbool.h>

uint64_t milliseconds();

//void init_timeout_ms(uint64_t *);

//bool is_timeout_ms(uint64_t *);

typedef struct timeout_t {
  uint32_t val_cnt_current;
  uint32_t timeout_ms;
} TIMEOUT_MS;

void init_timeout_ms(TIMEOUT_MS *, uint32_t);

bool is_timeout_ms(TIMEOUT_MS *);

#ifdef RTOS_SOLAR48

void vPortSetupTimerInterrupt();
void set_milliseconds_caller();

#else
#define delay_seconds(n) delay(1000*n)

void delay(uint64_t);

// https://github.com/mpaland/printf If needed install this library to access long int with small size
// Futher readings: https://metebalci.com/blog/demystifying-arm-gnu-toolchain-specs-nano-and-nosys/

#endif

void init_systick();

#endif

