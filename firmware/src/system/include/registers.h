#ifndef REGISTERS_H
  #define REGISTERS_H

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#include <stm32f103x6.h>
// Clock page 121
//#define RCC_BASE     0x40021000
//BEGIN Clock->RCC_CR (Page 99)
#define RCC_CR (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define PLLRDY (1<<25)
#define PLLON (1<<24)
#define CSSON (1<<19)
#define HSEBYP (1<<18)
#define HSERDY (1<<17)
#define HSEON (1<<16)
#define HSICAL(val) val<<8
#define HSITRIM(val) val<<3
#define HSIRDY (1<<1)
#define HSION (1<<0)
//END Clock->RCC_CR

//BEGIN Clock->RCC_CFGR (Page 101)
#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define MCO(val) (val<<24)
#define USBPRE (1<<23)
#define PLLMUL(val) (val<<18)
  #define PLLMULx9 0b0111

#define PLLXTPRE(val) (1<<17)
#define PLLSRC (1<<16)
#define ADCPRE(val) (val<<14)
#define PPRE2(val) (val<<11)
#define PPRE1(val) (val<<8)
  #define HCLK_div2 0b100
#define HPRE(val) (val<<4)
#define SWS(val) (val<<2)
  #define SWS_mask SWS(0b11)
  #define PLL_selected_as_system_clock SWS(0b10)
#define SW(val) (val<<0)
  #define PLL_as_system_clock 0b10
//END Clock->RCC_CFGR

//BEGIN Clock->RCC_CIR (Page 104)
#define RCC_CIR (*(volatile uint32_t *)(RCC_BASE + 0x08))
//TODO implement read/sets for RCC_CIR if needed
//END Clock->RCC_CIR (Page 104

//BEGIN Clock->RCC_APB2RSTR (Page 106)
#define RCC_APB2RSTR (*(volatile uint32_t *)(RCC_BASE + 0x0C))
//TODO implement read/sets for RCC_APB2RSTR if needed
//END Clock->RCC_APB2RSTR

//BEGIN Clock->RCC_APB1RSTR (Page 109)
#define RCC_APB1RSTR (*(volatile uint32_t *)(RCC_BASE + 0x10))
//TODO implement read/sets for RCC_APB1RSTR if needed
//END Clock->RCC_APB1RSTR

//BEGIN Clock->RCC_APB1RSTR (Page 111)
#define RCC_AHBENR (*(volatile uint32_t *)(RCC_BASE + 0x14))
//TODO implement read/sets for RCC_APB1RSTR if needed
//END Clock->RCC_APB1RSTR

//BEGIN Clock->RCC_APB2ENR (Page 112)
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define TIM11EN (1<<21)
#define TIM10EN (1<<20)
#define TIM9EN (1<<19)
#define ADC3EN (1<<15)
#define USART1EN (1<<14)
#define TIM8EN (1<<8)
#define SPI1EN (1<<12)
#define TIM1EN (1<<11)
#define ADC2EN (1<<10)
#define ADC1EN (1<<9)
#define IOPGEN (1<<8)
#define IOPFEN (1<<7)
#define IOPEEN (1<<6)
#define IOPDEN (1<<5)
#define IOPCEN (1<<4)
#define IOPBEN (1<<3)
#define IOPAEN (1<<2)
#define AFIOEN (1<<0)
//END Clock->RCC_APB2ENR

//BEGIN Clock->RCC_APB1ENR (Page 115)
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x1C))
#define USBEN (1<<23)
//TODO implement read/sets for RCC_APB1ENR if neede
//END Clock->RCC_APB1ENR

//END CLOCK

//GPIO
//#define GPIOC_BASE   0x40011000
#define GPIOC_CRH    (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR    (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#endif

