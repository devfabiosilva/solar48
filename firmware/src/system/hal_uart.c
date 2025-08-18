#include <registers.h>
#include <hal_uart.h>

void init_uart1()
{
  //TODO implement uart1 initialization
  USART1_BRR = UART1_DEFAULT_SPEED;
}

void USART1_IRQHandler()
{
  // TODO implement uart1 handler
}

