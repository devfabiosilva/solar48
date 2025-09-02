#include <registers.h>
#include <hal_uart.h>
#include <sys_interrupts.h>
#include <dma.h>
#include <solar48_config.h>
#include <time.h>

struct uart_control_t {
  volatile bool locked;                     // UART locked
  volatile uint8_t *data_ptr;               // Data pointer
  volatile uint64_t timeout;                // UART timeout
  volatile uint32_t status_register;        // UART status register
  volatile int32_t block;                   // Data is divided by n x UART1_TX_RX_BUF
  volatile int32_t left;                    // Data remaining left = data size % UART1_TX_RX_BUF
  void *uart_on_error_ctx;                  // Uart callback context
  uart_callback_func uart_on_error;         // Uart callback
  void *uart_receive_complete_ctx;          // Uart callback success
  uart_callback_func uart_receive_complete; // Uart callback success
};

volatile struct uart_control_t uart1_control = {0};

void USART1_IRQHandler()
{
  // TODO implement uart1 error handler
}

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785
static volatile uint8_t uart1_tx_rx[UART1_TX_RX_BUF];

void init_uart1()
{

  // 7.3.7 - APB2 peripheral clock enable register (RCC_APB2ENR) Page 112
  RCC_APB2ENR |= IOPAEN;   // Enables GPIOA. Page 113
  RCC_APB2ENR |= USART1EN; // Enables USART1. Page 113

  // ---- Configure GPIOA TX ----
  // PA9 = TX (AF push-pull), PA10 = RX (input floating)
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE9_VAL(0b11) | GPIOA_CNF9_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE9_VAL(0b11) | GPIOA_CNF9_VAL(0b10); // Output mode, max speed 50 MHz and Alternate function output Push-pull

  // ---- Configure GPIOA RX ----
  // PA9 = TX (AF push-pull), PA10 = RX (input floating)
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE10_VAL(0b11) | GPIOA_CNF10_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE10_VAL(0b00) | GPIOA_CNF10_VAL(0b01); // Floating input mode (reset state)

  // Set UART 1 Speed
  //See page 798: 27.3.4 Fractional baud rate generation
  USART1_BRR = UART1_DEFAULT_SPEED;

  USART1_CR3 = (
                  DMAT | // DMA enable transmitter
                  DMAR | // DMA enable receiver
                  EIE    // Error interrupt enable
               );

  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART1_CR1 = /*RE|*/TE;    // Enable receive/transmit
  USART1_CR1 |= IDLEIE;  // Enable idle interrupt
  USART1_CR1 |= UE;      // Enable UART1

  //dma1_channel4_init((void *)&uart1_tx_rx[0], sizeof(uart1_tx_rx), (void *)&USART1_DR);
  dma1_channel4_init((void *)&USART1_DR);
  //dma1_channel5_init((void *)&uart1_tx_rx[0], sizeof(uart1_tx_rx), (void *)&USART1_DR);

  __nvic_set_priority(USART1_IRQn, UART1_PRIO);
  __nvic_enable_irq(USART1_IRQn);

}

inline bool uart1_is_busy()
{
  return (((DMA1_CCR4 & DMA1_CCR4_EN) != 0) || ((USART1_CR1 & RE) != 0));
}

enum uart_status_t uart1_transmit(
  uint8_t *data, size_t data_size,
  uart_callback_func uart_receive_complete,
  void *uart_receive_complete_ctx,
  uart_callback_func uart_on_error,
  void *uart_on_error_ctx
)
{
  if (uart1_is_busy())
    return UART_BUSY;

  if (uart1_control.locked)
    return UART_LOCKED;

  uart1_control.locked = true;

  DMA1_CCR4 &= ~(DMA1_CCR4_EN);

  DMA1_CMAR4 = (uint32_t)data; // Memory address Page 288

  uart1_control.data_ptr = data;
  init_timeout_ms((uint64_t *)&uart1_control.timeout);
  uart1_control.status_register = 0;
  uart1_control.uart_on_error_ctx = uart_on_error_ctx;
  uart1_control.uart_on_error = uart_on_error;
  uart1_control.uart_receive_complete = uart_receive_complete;
  uart1_control.uart_receive_complete_ctx = uart_receive_complete_ctx;

  int32_t block = (int32_t)data_size / UART1_TX_RX_BUF;
  int32_t left = (int32_t)data_size % UART1_TX_RX_BUF;

  if (block > 0) {
    DMA1_CNDTR4 = (uint16_t)UART1_TX_RX_BUF; // Memory size Page 287 (64 KB max)
    --block;
  } else
    DMA1_CNDTR4 = (uint16_t)left; // Memory size Page 287 (64 KB max)

  uart1_control.block = block;
  uart1_control.left = left;

  DMA1_CCR4 |= DMA1_CCR4_EN;

  return UART_OK;
}

