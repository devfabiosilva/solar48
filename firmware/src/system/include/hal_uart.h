#ifndef HAL_UART_H
 #define HAL_UART_H

//27.5 USART mode configuration page 817
//2.2 List of abbreviations for registers page 45

#define UART_SPEED(mantissa, fractional) (mantissa << 4) | fractional

//27.3.4 Fractional baud rate generation: Page 798 | Table 192. Error calculation for programmed baud rates: Page 799
//PCLK2 = 72MHz for UART1 and PCLK1 = 36MHz for UART2

//UART1 @ PCLK2 = 72 MHz
#define UART1_2_4_KBPS      UART_SPEED(1875, 0)  // 2.4 kbps
#define UART1_9_6_KBPS      UART_SPEED(468, 12)  // 9.6 kbps
#define UART1_19_2_KBPS     UART_SPEED(234, 6)   // 19.2 kbps
#define UART1_57_6_KBPS     UART_SPEED(78, 2)    // 57.6 kbps
#define UART1_115_2_KBPS    UART_SPEED(39, 1)    // 115.2 kbps
#define UART1_230_769_KBPS  UART_SPEED(19, 8)    // 230.769 kbps
#define UART1_461_538_KBPS  UART_SPEED(9, 12)    // 461.538 kbps
#define UART1_923_076_KBPS  UART_SPEED(4, 14)    // 923.076 kbps
#define UART1_2250_KBPS     UART_SPEED(2, 0)     // 2250 kbps
#define UART1_4500_KBPS     UART_SPEED(1, 0)     // 4500 kbps
#define UART1_DEFAULT_SPEED UART1_19_2_KBPS

//UART2 @ PCLK1 = 36 MHz
#define UART2_2_4_KBPS      UART_SPEED(937, 8)   // 2.4 kbps
#define UART2_9_6_KBPS      UART_SPEED(234, 6)   // 9.6 kbps
#define UART2_19_2_KBPS     UART_SPEED(117, 3)   // 19.2 kbps
#define UART2_57_6_KBPS     UART_SPEED(39, 1)    // 57.6 kbps
#define UART2_115_2_KBPS    UART_SPEED(19, 8)    // 115.2 kbps
#define UART2_230_769_KBPS  UART_SPEED(9, 12)    // 230.769 kbps
#define UART2_461_538_KBPS  UART_SPEED(4, 14)    // 461.538 kbps
#define UART2_923_076_KBPS  UART_SPEED(2, 7)     // 923.076 kbps
#define UART2_2250_KBPS     UART_SPEED(1, 0)    // 2250 kbps
#define UART2_DEFAULT_SPEED UART2_19_2_KBPS
//#define UART2_4500_KBPS     N/A

void init_uart1();

#endif

