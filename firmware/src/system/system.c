//ter 20 mai 2025 19:28:25 
#include <registers.h>

void system_init(void)
{
// TODO initialize clock system here
    RCC_APB2ENR |= (1 << 4);
}

