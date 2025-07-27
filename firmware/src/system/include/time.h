#ifndef TIME_H
 #define TIME_H

uint64_t milliseconds();

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

//TODO remove it. For tests only. Delegate it to FreeRTOS
void delay_5us();

#endif

