#include <registers.h>

static void delay(volatile int count)
{
  while((--count) > 0) __asm__("nop");
}

void delay_test() {
  while(1)
  {
    GPIOC_ODR &= ~(1 << 13); // LED ligado (nível baixo no PC13)
    delay(500000);
    GPIOC_ODR |= (1 << 13);  // LED desligado
    delay(500000);
  }
}

