#ifndef TIME_H
 #define TIME_H

uint64_t milliseconds();
void init_systick();

#ifdef RTOS_SOLAR48

int has_rtos_ticks();

#else
#define delay_seconds(n) delay(1000*n)

void delay(uint64_t);

//TODO remove it. For tests only. Delegate it to FreeRTOS
void delay_5us();

// https://github.com/mpaland/printf If needed install this library to access long int with small size
// Futher readings: https://metebalci.com/blog/demystifying-arm-gnu-toolchain-specs-nano-and-nosys/

#endif

#endif

