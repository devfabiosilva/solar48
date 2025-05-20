#include <registers.h>

void init_gpios(void)
{
    // Configurar PC13 como saída push-pull 2MHz
    GPIOC_CRH &= ~(0xF << 20); // Limpar bits MODE13 e CNF13
    GPIOC_CRH |=  (0x2 << 20); // MODE13 = 0b10 (Output 2 MHz)
}

