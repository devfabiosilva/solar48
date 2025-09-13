#ifndef REGISTERS_H
  #define REGISTERS_H

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
//RCC_BDCR

#include <stm32f103x6.h>
// Clock page 121
//RCC BASE REGISTER MAP PAG 121
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
#define ADCPRE(val) (val<<14)
  #define PCLK2_divided_by_6 0b10 // 12MHz for ADC converter with ABP2 = 72MHz
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
  #define I2C1RST (1<<21)
//END Clock->RCC_APB1RSTR

//BEGIN Clock->RCC_APB1RSTR (Page 111)
#define RCC_AHBENR (*(volatile uint32_t *)(RCC_BASE + 0x14))
  #define DMA1EN (1<<0)
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
#define ADC1EN (1<<9)
//END Clock->RCC_APB2ENR

//BEGIN Clock->RCC_APB1ENR (Page 115)
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x1C))
 #define USBEN (1<<23)
 #define PWREN (1<<28)
 #define I2C1EN (1<<21)
//TODO implement read/sets for RCC_APB1ENR if needed
//END Clock->RCC_APB1ENR

// RTC Clock (Page 118)
#define RCC_BDCR  (*(volatile uint32_t *)(RCC_BASE + 0x20))
 #define RTC_SEL_mask (0x03<<8)
 #define LSE_oscillator_clock_used_as_RTC_clock ((0x01)<<8)
 #define LSEON (1<<0)
 #define LSERDY (1<<1)
 #define RTCEN (1<<15)
 #define BDRST (1<<16)
//END CLOCK

//Power control register PAGE 77 
#define PWR_CR (*(volatile uint32_t *)(PWR_BASE + 0x00))
  #define DBP (1<<8)

//RTC REGISTERS Page 487
#define RTC_CRH (*(volatile uint16_t *)(RTC_BASE + 0x00))
  #define SECIE (1<<0)

#define RTC_CRL (*(volatile uint16_t *)(RTC_BASE + 0x04)) // Page 488
  #define RTOFF (1<<5)
  #define CNF (1<<4)
  #define RSF (1<<3)
  #define OWF (1<<2)
  #define ALRF (1<<1)
  #define SECF (1<<0)

#define RTC_PRLH (*(volatile uint16_t *)(RTC_BASE + 0x08)) //Page 489
  #define RTC_PRLH_mask (0x0F)

#define RTC_PRLL (*(volatile uint16_t *)(RTC_BASE + 0x0C)) //Page 490

#define RTC_DIVH (*(volatile uint16_t *)(RTC_BASE + 0x10)) //Page 490
    #define RTC_DIVH_mask 0x0F

#define RTC_DIVL (*(volatile uint16_t *)(RTC_BASE + 0x14)) //Page 490

#define RTC_CNTH (*(volatile uint16_t *)(RTC_BASE + 0x18)) //Page 491
#define RTC_CNTL (*(volatile uint16_t *)(RTC_BASE + 0x1C)) //Page 491

//9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
#define GPIOA_CRH (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
 #define GPIOA_MODE9_VAL(val) (val<<4)
 #define GPIOA_CNF9_VAL(val) (val<<6)
 #define GPIOA_MODE10_VAL(val) (val<<8)
 #define GPIOA_CNF10_VAL(val) (val<<10)

//GPIO
//#define GPIOC_BASE   0x40011000
#define GPIOC_CRH    (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR    (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#define GPIOB_CRL (*(volatile uint32_t *)(GPIOB_BASE + 0x00)) //page 171 9.2.1
 #define GPIOB_MODE6_VAL(val) (val<<24)
 #define GPIOB_MODE7_VAL(val) (val<<28)
 #define GPIOB_CNF6_VAL(val) (val<<26)
 #define GPIOB_CNF7_VAL(val) (val<<30)

// Independent Watchdog configuration: Page 496
#define IWDG_KR (*(volatile uint32_t *)(IWDG_BASE + 0x00))
#define IWDG_PR (*(volatile uint32_t *)(IWDG_BASE + 0x04))
  #define PR_divide_by_256 (0b111) // Page 497

#define IWDG_RLR (*(volatile uint32_t *)(IWDG_BASE + 0x08))

//Control/status register (RCC_CSR) Page 119
#define RCC_CSR (*(volatile uint32_t *)(RCC_BASE + 0x24))
  #define LSION (1<<0)
  #define LSIRDY (1<<1)
  #define IWDGRSTF (1<<29)
  #define RMVF (1<<24)


// CHIP UID
// MEMORY SIZE Page 1076
#define FLASH_SIZE_REGISTER (*(volatile uint16_t *)(0x1FFFF7E0))

//Page 1077
#define CHIP_UID_BASE_ADDR (uint32_t)0x1FFFF7E8
#define CHIP_UID_BASE_0 (*(volatile uint16_t *)(CHIP_UID_BASE_ADDR + 0x00))
#define CHIP_UID_BASE_1 (*(volatile uint16_t *)(CHIP_UID_BASE_ADDR + 0x02))
#define CHIP_UID_BASE_2 (*(volatile uint32_t *)(CHIP_UID_BASE_ADDR + 0x04))
#define CHIP_UID_BASE_3 (*(volatile uint32_t *)(CHIP_UID_BASE_ADDR + 0x08))

// ADC conversion page 238
#define ADC1_SR (*(volatile uint32_t *)(ADC1_BASE + 0x00))
  #define EOC (1<<1)
#define ADC1_CR1 (*(volatile uint32_t *)(ADC1_BASE + 0x04))
#define ADC1_CR2 (*(volatile uint32_t *)(ADC1_BASE + 0x08))
  #define TSVREFE (1<<23)
  #define ADON (1<<0)

//#define ADC_SQR1 (*(volatile uint32_t *)(ADC_BASE + 0x2C))

#define ADC1_SQR3 (*(volatile uint32_t *)(ADC1_BASE + 0x34))
#define ADC1_DR (*(volatile uint32_t *)(ADC1_BASE + 0x4C))
// END OF ADC conversion

//NVIC Parameters
void __nvic_enable_irq(IRQn_Type IRQn);

/** @defgroup CORTEX_Preemption_Priority_Group CORTEX Preemption Priority Group
  * @{
  */
#define NVIC_PRIORITYGROUP_0         0x00000007U /*!< 0 bits for pre-emption priority
                                                      4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         0x00000006U /*!< 1 bits for pre-emption priority
                                                      3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         0x00000005U /*!< 2 bits for pre-emption priority
                                                      2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         0x00000004U /*!< 3 bits for pre-emption priority
                                                      1 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         0x00000003U /*!< 4 bits for pre-emption priority
                                                      0 bits for subpriority */

void __nvic_setprioritygrouping(uint32_t);
void __nvic_set_priority(IRQn_Type, uint32_t);
/**
  \brief   Get Priority Grouping
  \details Reads the priority grouping field from the NVIC Interrupt Controller.
  \return                Priority grouping field (SCB->AIRCR [10:8] PRIGROUP field).
 */
uint32_t get_priority_grouping();
void __nvic_disable_irq(IRQn_Type);

#define I2C1_CR1 (*(volatile uint16_t *)(I2C1_BASE + 0x00)) // Page 772: 26.6.1
 #define SWRST (1<<15)
 #define POS (1<<11)
 #define START (1<<8)
 #define STOP (1<<9)
 #define NOSTRETCH (1<<7)
 #define PE (1<<0)
#define I2C1_CCR (*(volatile uint16_t *)(I2C1_BASE + 0x1C)) // Page 768: 26.6.8
 #define FS (1<<15)
#define I2C1_TRISE (*(volatile uint16_t *)(I2C1_BASE + 0x20)) // Page 782: 26.6.9
#define I2C1_CR2 (*(volatile uint16_t *)(I2C1_BASE + 0x04)) // Page 774: 26.6.2
 #define ITBUFEN (1<<10)
 #define ITEVTEN (1<<9)
 #define ITERREN (1<<8)

#define I2C1_SR1 (*(volatile uint16_t *)(I2C1_BASE + 0x14)) // Page 777: 26.6.6
 #define AF (1<<10)
 #define TxE (1<<7)
 #define BTF (1<<2)
 #define ADDR (1<<1)
 #define SB (1<<0)

#define I2C1_SR2 (*(volatile uint16_t *)(I2C1_BASE + 0x18)) // Page 780: 26.6.7
 #define BUSY (1<<1)

#define I2C1_DR (*(volatile uint16_t *)(I2C1_BASE + 0x10)) // Page 777: 26.6.5

#define __HAL_RCC_GPIOB_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);\
                                        /* Delay after an RCC peripheral clock enabling */\
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_I2C1_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        /* Delay after an RCC peripheral clock enabling */\
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_AFIO_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);\
                                        /* Delay after an RCC peripheral clock enabling */\
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)


#define CURRENT_TIMESTAMP ((RTC_CNTH<<16)|(RTC_CNTL))

//USART1_BASE
#define USART1_SR (*(volatile uint32_t *)(USART1_BASE + 0x00)) // Page 818: 27.6.1
  #define CTS (1<<9)
  #define RXNE (1<<5)
  #define TC (1<<6)
  #define ORE (1<<3)
  #define NE (1<<2)
  #define FE (1<<1)
  #define PE (1<<0)

#define USART1_DR (*(volatile uint32_t *)(USART1_BASE + 0x04)) // Page 820: 27.6.2

// See page 798: 27.3.4 Fractional baud rate generation
#define USART1_BRR (*(volatile uint32_t *)(USART1_BASE + 0x08)) // 27.6.3 Baud rate register (USART_BRR) page 820

#define USART1_CR1 (*(volatile uint32_t *)(USART1_BASE + 0x0C)) //27.6.4 Control register 1 (USART_CR1) page 821
  #define RE (1<<2)
  #define TE (1<<3)
  #define IDLEIE (1<<4)
  #define RXNEIE (1<<5)
  #define TCIE (1<<6)
  #define TXEIE (1<<7)
  #define PEIE (1<<8)
  #define PCE (1<<10)
  #define M (1<<12)
  #define UE (1<<13)

// 27.6.5 Control register 2 (USART_CR2) page 823
#define USART1_CR2 (*(volatile uint32_t *)(USART1_BASE + 0x10))

//27.6.6 Control register 3 (USART_CR3) page 824
#define USART1_CR3 (*(volatile uint32_t *)(USART1_BASE + 0x14))
  #define EIE (1<<0)
  #define NACK (1<<4)
  #define DMAT (1<<7)
  #define DMAR (1<<8)

//13.4.3 DMA channel x configuration register (DMA_CCRx) (x = 1..7, where x = channel number) Page 286
#define DMA1_CCRx(x) (*(volatile uint32_t *)(DMA1_BASE + 0x08 + 20*(x - 1)))

//13.4.5 DMA channel x peripheral address register (DMA_CPARx) (x = 1..7, where x = channel number) Page 288
#define DMA1_CPARx(x) (*(volatile uint32_t *)(DMA1_BASE + 0x10 + 20*(x - 1)))

//13.4.6 DMA channel x memory address register (DMA_CMARx) (x = 1..7, where x = channel number) Page 288
#define DMA1_CMARx(x) (*(volatile uint32_t *)(DMA1_BASE + 0x14 + 20*(x - 1)))

//13.4.4 DMA channel x number of data register (DMA_CNDTRx) (x = 1..7, where x = channel number) Page 287
#define DMA1_CNDTRx(x) (*(volatile uint32_t *)(DMA1_BASE + 0x0C + 20*(x - 1)))

// DMA1 CHANNEL 4
#define DMA1_CCR4 DMA1_CCRx(4)
  #define DMA1_CCR4_EN (1<<0)
  #define DMA1_TCIE4 (1<<1)
  #define DMA1_TEIE4 (1<<3)
  #define DMA1_DIR4 (1<<4)
  #define DMA1_MINC4 (1<<7)
  #define DMA1_PL4_SEL(val) (val<<12)

#define DMA1_CPAR4 DMA1_CPARx(4)
#define DMA1_CMAR4 DMA1_CMARx(4)
#define DMA1_CNDTR4 DMA1_CNDTRx(4)

// DMA1 CHANNEL 5
#define DMA1_CCR5 DMA1_CCRx(5)
  #define DMA1_CCR5_EN (1<<0)
  #define DMA1_TCIE5 (1<<1)
  #define DMA1_TEIE5 (1<<3)
  #define DMA1_DIR5 (1<<4)
  #define DMA1_MINC5 (1<<7)
  #define DMA1_PL5_SEL(val) (val<<12)

#define DMA1_CPAR5 DMA1_CPARx(5)
#define DMA1_CMAR5 DMA1_CMARx(5)
#define DMA1_CNDTR5 DMA1_CNDTRx(5)

//13.4.1 DMA interrupt status register (DMA_ISR) Page 284
#define DMA1_ISR (*(volatile uint32_t *)(DMA1_BASE + 0x00))
  #define GIF4 (1<<12)
  #define TCIF4 (1<<13)
  #define HTIF4 (1<<14)
  #define TEIF4 (1<<15)

  #define GIF5 (1<<16)
  #define TCIF5 (1<<17)
  #define HTIF5 (1<<18)
  #define TEIF5 (1<<19)

//13.4.2 DMA interrupt flag clear register (DMA_IFCR) Page 285
#define DMA1_IFCR (*(volatile uint32_t *)(DMA1_BASE + 0x04))
  #define CGIF4 (1<<12)
  #define CTCIF4 (1<<13)
  #define CHTIF4 (1<<14)
  #define CTEIF4 (1<<15)

  #define CGIF5 (1<<16)
  #define CTCIF5 (1<<17)
  #define CHTIF5 (1<<18)
  #define CTEIF5 (1<<19)

#endif

