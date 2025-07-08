#ifndef SYS_INTERRUPTS_H
 #define SYS_INTERRUPTS_H

#ifdef RTOS_SOLAR48
 #define USB_PRIO     12
 #define SYSTICK_PRIO 14
 #define RTC_PRIO     13
 #define PEND_SV_PRIO 15
 #define PEND_SV_CALL 0

#else

 #define USB_PRIO 5
 #define SYSTICK_PRIO 6
 #define RTC_PRIO 7

#endif

#endif
