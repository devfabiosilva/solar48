#ifndef SYSTICK_CONFIG_H
 #define SYSTICK_CONFIG_H

#define SYS_TICK_FREQ_HZ 1000UL  // 1ms
#define CPU_FREQ_HZ      72000000UL 
/*
#ifdef RTOS_SOLAR48
  #define SYS_TICK_DIV 1 // For FreeRTOS
#else
  #define SYS_TICK_DIV     8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk
#endif
*/

#define SYS_TICK_DIV 8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk
#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)

_Static_assert(SYSTICK_TICKS <= 0xFFFFFF, "SYSTICK_TICKS too high for SysTick LOAD register!");

#endif

