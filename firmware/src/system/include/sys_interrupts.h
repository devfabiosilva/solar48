#ifndef SYS_INTERRUPTS_H
 #define SYS_INTERRUPTS_H

#ifdef RTOS_SOLAR48
 #define RTC_PRIO     9
 #define USB_PRIO     10
 #define I2C1_EV_PRIO 11
 #define I2C1_ER_PRIO 12
 #define SYSTICK_PRIO 14
 #define PEND_SV_PRIO 15
 #define PEND_SV_CALL 0

#else

 #define SYSTICK_PRIO  1
 #define RTC_PRIO      2
 #define USB_PRIO      3
 #define I2C1_EV_PRIO  4
 #define I2C1_ER_PRIO  5

#endif

#endif
