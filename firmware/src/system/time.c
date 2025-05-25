#include <registers.h>

static void _delay(volatile int count)
{
  while((--count) > 0) __asm__("nop");
}
#define T 7200000

void delay_test() {
  while(1)
  {
    //GPIOC_ODR &= ~(1 << 13); // LED ligado (nível baixo no PC13)
    _delay(T);
    //GPIOC_ODR |= (1 << 13);  // LED desligado
    _delay(T);
  }
}

void delay_seconds(int n) {
  int k = n;
  while (k > 0) {
    _delay(T);
    k--;
  }
}

