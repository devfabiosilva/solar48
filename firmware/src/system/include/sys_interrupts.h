#ifndef SYS_INTERRUPTS_H
 #define SYS_INTERRUPTS_H

// Note: In ARM priority if lower value, higher priority

#ifdef RTOS_SOLAR48

 #define RTC_PRIO     8
 #define UART1_PRIO   9
 #define USB_PRIO     10
 #define I2C1_EV_PRIO 11
 #define I2C1_ER_PRIO 12
 #define SYSTICK_PRIO 14 // In RTOS SYSTICK must have lower priority than others interrupts
 #define PEND_SV_PRIO 15
 #define PEND_SV_CALL 0  // This must be always 0 for RTOS (higher priority)

#else

 #define SYSTICK_PRIO  1
 #define RTC_PRIO      2
 #define UART1_PRIO    3
 #define USB_PRIO      4
 #define I2C1_EV_PRIO  5
 #define I2C1_ER_PRIO  6

#endif

#endif
