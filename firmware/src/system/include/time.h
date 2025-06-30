#ifndef TIME_H
 #define TIME_H
//void delay_test();
#define delay_seconds(n) delay(1000*n)
uint64_t milliseconds();
void delay(uint64_t);
void init_systick();

//TODO remove it. For tests only. Delegate it to FreeRTOS
void delay_5us();

// https://github.com/mpaland/printf If needed install this library to access long int with small size
// Futher readings: https://metebalci.com/blog/demystifying-arm-gnu-toolchain-specs-nano-and-nosys/
#endif

