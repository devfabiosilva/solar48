#ifndef SOLAR48_CONFIG_H
 #define SOLAR48_CONFIG_H

// **** Clock and SysTick system clocks ****

#define SYS_TICK_FREQ_HZ 1000UL  // in ticks per seconds
#define CPU_FREQ_HZ      72000000UL // System core frequency in Hertz

#define SYS_TICK_DIV 8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk or 1 if no prescale systick timer (enable SysTick_CTRL_CLKSOURCE_Msk)
#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)

_Static_assert(SYSTICK_TICKS <= 0xFFFFFF, "SYSTICK_TICKS too high for SysTick LOAD register");

// *****************************************

// ****      USB CDC configuration      ****
/* Define size for the receive and transmit buffer over CDC */
#define APP_RX_DATA_SIZE  256
#define APP_TX_DATA_SIZE  256
// *****************************************


#endif

