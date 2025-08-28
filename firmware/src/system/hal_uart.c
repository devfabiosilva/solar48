#include <registers.h>
#include <hal_uart.h>
#include <sys_interrupts.h>
#include <dma.h>
#include <solar48_config.h>

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
  USART1_CR1 = RE|TE;    // Enable receive/transmit
  USART1_CR1 |= IDLEIE;  // Enable idle interrupt
  USART1_CR1 |= UE;      // Enable UART1

  dma1_channel4_init((void *)&uart1_tx_rx[0], sizeof(uart1_tx_rx), (void *)&USART1_DR);
  dma1_channel5_init((void *)&uart1_tx_rx[0], sizeof(uart1_tx_rx), (void *)&USART1_DR);

  __nvic_set_priority(USART1_IRQn, UART1_PRIO);
  __nvic_enable_irq(USART1_IRQn);

}

