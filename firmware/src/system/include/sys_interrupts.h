#ifndef SYS_INTERRUPTS_H
 #define SYS_INTERRUPTS_H

// Note: In ARM priority if lower value, higher priority

#ifdef RTOS_SOLAR48

 #define PEND_SV_CALL  0              // This must be always 0 for RTOS (higher priority)
 #define TIM2_PRIO     5
 #define TIM3_PRIO     TIM2_PRIO
 #define RTC_PRIO      6              // Internal interrupt has more priority than external interrupt
 #define DMA1_CH4_PRIO 7              // Used in UART1 TX for ModBus Master
 #define DMA1_CH5_PRIO DMA1_CH4_PRIO  // Used in UART1 RX for ModBus Master
 #define DMA1_CH6_PRIO 8              // Used in UART2 RX for ModBus Slave
 #define DMA1_CH7_PRIO DMA1_CH6_PRIO  // Used in UART2 TX for ModBus Slave
 #define UART1_PRIO    9
 #define UART2_PRIO    10
 #define USB_PRIO      11
 #define I2C1_EV_PRIO  12
 #define I2C1_ER_PRIO  13
 #define SYSTICK_PRIO  14             // In RTOS SYSTICK must have lower priority than others interrupts
 #define PEND_SV_PRIO  15

#else

 #define TIM2_PRIO     1
 #define TIM3_PRIO     TIM2_PRIO
 #define SYSTICK_PRIO  2
 #define RTC_PRIO      3
 #define DMA1_CH4_PRIO 4
 #define DMA1_CH5_PRIO DMA1_CH4_PRIO
 #define DMA1_CH6_PRIO 5
 #define DMA1_CH7_PRIO DMA1_CH6_PRIO
 #define UART1_PRIO    6
 #define UART2_PRIO    7
 #define USB_PRIO      8
 #define I2C1_EV_PRIO  9
 #define I2C1_ER_PRIO  10

#endif

#endif

