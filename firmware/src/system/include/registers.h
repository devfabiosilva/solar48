#ifndef REGISTERS_H
  #define REGISTERS_H

#include <stdint.h>

// Clock
#define RCC_BASE     0x40021000
#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x18))

//GPIO
#define GPIOC_BASE   0x40011000
#define GPIOC_CRH    (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR    (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#endif

