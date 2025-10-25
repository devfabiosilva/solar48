#ifndef HAL_UART_H
 #define HAL_UART_H

#include <stddef.h>
#include <stdbool.h>
#include <errors.h>
#include <solar48_config.h>
#include <registers.h>

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

#define UART1_PARITY_MODE_DISABLE 0
#define UART1_PARITY_MODE_ODD (M|PCE|PEIE)
#define UART1_PARITY_MODE_EVEN (UART1_PARITY_MODE_ODD|PS)

#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
enum uart1_speed_e {
  speed_2_4_kbps = UART1_2_4_KBPS,
  speed_9_6_kbps = UART1_9_6_KBPS,
  speed_19_2_kpbs = UART1_19_2_KBPS,
  speed_57_6_kbps = UART1_57_6_KBPS,
  speed_115_2_kbps = UART1_115_2_KBPS,
  speed_230_769_kbps = UART1_230_769_KBPS,
  speed_461_538_kbps = UART1_461_538_KBPS,
  speed_923_076_kbps = UART1_923_076_KBPS,
  speed_2250_kbps = UART1_2250_KBPS,
  speed_4500_kbps = UART1_4500_KBPS
};

enum uart1_mode_e {
  PARITY_DISABLE = UART1_PARITY_MODE_DISABLE,
  PARITY_ODD = UART1_PARITY_MODE_ODD,
  PARITY_EVEN = UART1_PARITY_MODE_EVEN
};

 #define UART1_DEFAULT_SPEED speed_115_2_kbps

void init_uart1(enum uart1_speed_e speed, enum uart1_mode_e);

#else

enum rs485_master_speed_e {
  speed_2_4_kbps = 0,
  speed_9_6_kbps,
  speed_19_2_kpbs,
  speed_57_6_kbps,
  speed_115_2_kbps,
  speed_230_769_kbps,
  speed_461_538_kbps,
  speed_923_076_kbps,
  speed_2250_kbps,
  speed_4500_kbps
};

enum rs485_master_mode_e {
  PARITY_DISABLE = UART1_PARITY_MODE_DISABLE,
  PARITY_ODD = UART1_PARITY_MODE_ODD,
  PARITY_EVEN = UART1_PARITY_MODE_EVEN
};

 #define RS485_MASTER_DEFAULT_SPEED speed_115_2_kbps

void init_master_rs485(enum rs485_master_speed_e speed, enum rs485_master_mode_e);

#endif

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
#define UART2_DEFAULT_SPEED UART2_115_2_KBPS
//#define UART2_4500_KBPS     N/A

typedef void (*uart_callback_func)(int);

enum uart_status_t {
  UART_OK = 0,
  UART_BUSY = E_UART1_BUSY,
  UART_LOCKED = E_UART1_LOCKED
};

enum uart_status_t uart1_transmit(
  uint8_t *, size_t,
  uart_callback_func,
  uint32_t
);

#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
enum uart_status_t uart1_receive(
  uint8_t *, size_t,
  uart_callback_func,
  uint32_t
);
#endif

bool uart1_is_busy();

#ifndef RTOS_SOLAR48
void process_uart1_time_event();
#endif

#define UART1_TRANSFER_COMPLETE 1
#define UART1_RECEIVE_COMPLETE 2

#endif

