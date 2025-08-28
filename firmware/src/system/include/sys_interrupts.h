#ifndef SYS_INTERRUPTS_H
 #define SYS_INTERRUPTS_H

// Note: In ARM priority if lower value, higher priority

#ifdef RTOS_SOLAR48

 #define PEND_SV_CALL  0              // This must be always 0 for RTOS (higher priority)
 #define RTC_PRIO      8              // Internal interrupt has more priority than external interrupt
 #define DMA1_CH4_PRIO 9              // Used in UART1 TX for ModBus Master
 #define DMA1_CH5_PRIO DMA1_CH4_PRIO  // Used in UART1 RX for ModBus Master
 #define UART1_PRIO    10
 #define USB_PRIO      11
 #define I2C1_EV_PRIO  12
 #define I2C1_ER_PRIO  13
 #define SYSTICK_PRIO  14             // In RTOS SYSTICK must have lower priority than others interrupts
 #define PEND_SV_PRIO  15

#else

 #define SYSTICK_PRIO  1
 #define RTC_PRIO      2
 #define DMA1_CH4_PRIO 3
 #define DMA1_CH5_PRIO DMA1_CH4_PRIO
 #define UART1_PRIO    4
 #define USB_PRIO      5
 #define I2C1_EV_PRIO  6
 #define I2C1_ER_PRIO  7

#endif

#endif

