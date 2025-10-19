#include <registers.h>

#ifdef RTOS_SOLAR48
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#define delay_seconds(n) vTaskDelay(pdMS_TO_TICKS(1000*n))

#else

#include <time.h>

#endif

void init_gpios(void)
{
    RCC_APB2ENR |= IOPCEN;
    // Configure PC13 as output push-pull 2MHz
    GPIOC_CRH &= ~(0xF << 20); // Clear bits MODE13 e CNF13
    GPIOC_CRH |=  (0x2 << 20); // MODE13 = 0b10 (Output 2 MHz)
}

// TODO remove. For testing only
void ledon()
{
  GPIOC_ODR &= ~(1 << 13); // LED on (low level no PC13)
}

// TODO remove. For testing only
void ledoff()
{
 GPIOC_ODR |= (1 << 13);  // LED off
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

