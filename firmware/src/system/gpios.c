#include <registers.h>
#include <time.h>

void init_gpios(void)
{
    // Configurar PC13 como saída push-pull 2MHz
    GPIOC_CRH &= ~(0xF << 20); // Limpar bits MODE13 e CNF13
    GPIOC_CRH |=  (0x2 << 20); // MODE13 = 0b10 (Output 2 MHz)
}

void ledon()
{
  GPIOC_ODR &= ~(1 << 13); // LED ligado (nível baixo no PC13)
}

void ledoff()
{
 GPIOC_ODR |= (1 << 13);  // LED desligado
}

void blink_n(int n)
{
  int k = n;
  while (k > 0) {
    delay_seconds(1);
    ledon();
    delay_seconds(1);
    ledoff();
    k--;
  }
}
