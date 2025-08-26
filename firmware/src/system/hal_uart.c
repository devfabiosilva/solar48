#include <registers.h>
#include <hal_uart.h>
#include <sys_interrupts.h>

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785
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

  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART1_CR1 = RE|TE;    // Enable receive/transmit
  USART1_CR1 |= RXNEIE;  // Enable receive
  USART1_CR1 |= UE;      // Enable UART1

  __nvic_set_priority(USART1_IRQn, UART1_PRIO);
  __nvic_enable_irq(USART1_IRQn);

}

void USART1_IRQHandler()
{
  // TODO implement uart1 handler
}

