// TODO implement i2c bare metal ports peripheral
//Inter-integrated circuit (I2C) interface page 752

#include <stdint.h>
#include <stddef.h>
#include <registers.h>
#include <sys_interrupts.h>
#include <hal_i2c.h>

#define PCLK1_FREQ_IN_MHZ 36

extern uint64_t milliseconds();

void hal_i2c1_init2()
{

  RCC_APB2ENR |= IOPBEN; // IO port B clock enable page 114

  //I2C1 (PB6=SCL, PB7=SDA)
  GPIOB_CRL &= ~(GPIOB_MODE6_VAL(0b11) | GPIOB_MODE7_VAL(0b11) | GPIOB_CNF6_VAL(0b11) | GPIOB_CNF7_VAL(0b11)); // Clear PB6 and PB7 before configure these 2 pins. Page 171

  //11: Output mode, max speed 50 MHz. and 11: Alternate function output Open-drain: Page 171
  GPIOB_CRL |=  (GPIOB_MODE6_VAL(0b11) | GPIOB_MODE7_VAL(0b11) | GPIOB_CNF6_VAL(0b11) | GPIOB_CNF7_VAL(0b11));

  // fPCLK1 = 36MHz or TPCLK1 = 27.78ns
  RCC_APB1ENR |= I2C1EN; // I2C1 clock enable, page 116

  // According to page 778 I2C_CCR = 180, thus 180 x 27,78ns ~ 5000ns to allow 100KHz SCL (Slow mode)
  I2C1_CCR = 180;

  // According to page 782 I2C1_TRISE = 37 (36 + 1), for maximum allowed at fPCLK1 = 36MHz
  // Note: TRISE[5:0] = 2 in reset value
  I2C1_TRISE = PCLK1_FREQ_IN_MHZ + 1;

  //26.6.2 I2C Control register 2 (I2C_CR2) page 774
  I2C1_CR2 = PCLK1_FREQ_IN_MHZ;
  //I2C1_CR2 = ITBUFEN|ITEVTEN|ITERREN|PCLK1_FREQ_IN_MHZ; // TODO improve event handlers
/*
  //TODO Enable interrupts when implement event handlers
  __nvic_set_priority(I2C1_EV_IRQn, I2C1_EV_PRIO);
  __nvic_enable_irq(I2C1_EV_IRQn);
  __nvic_set_priority(I2C1_ER_IRQn, I2C1_ER_PRIO);
  __nvic_enable_irq(I2C1_ER_IRQn);
*/
  I2C1_CR1 |= PE;
}

// Important in 2 https://electronics.stackexchange.com/questions/272427/stm32-busy-flag-is-set-after-i2c-initialization
/*
Page 162
It is also possible to emulate the AFI input pin by software by programming the GPIO
controller. In this case, the port should be configured in Alternate Function Output mode.
And obviously, the corresponding port should not be driven externally as it will be driven by
the software using the GPIO controller.

For alternate function outputs, the port must be configured in Alternate Function Output
mode (Push-Pull or Open-Drain).
For bidirectional Alternate Functions, the port bit must be configured in Alternate
Function Output mode (Push-Pull or Open-Drain). In this case the input driver is
configured in input floating mode
If a port bit is configured as Alternate Function Output, this disconnects the output register
and connects the pin to the output signal of an on-chip peripheral.
If software configures a GPIO pin as Alternate Function Output, but peripheral is not
activated, its output is not specified.
*/
//https://electronics.stackexchange.com/questions/482111/stm32-i2c-master-does-not-generate-start-bit
//libopencm3
void hal_i2c1_init()
{

  //RCC_APB2ENR |= IOPBEN; // IO port B clock enable page 114
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // fPCLK1 = 36MHz or TPCLK1 = 27.78ns
  //RCC_APB1ENR |= I2C1EN; // I2C1 clock enable, page 116
  __HAL_RCC_I2C1_CLK_ENABLE();

  __HAL_RCC_AFIO_CLK_ENABLE();

  //I2C1 (PB6=SCL, PB7=SDA)
  GPIOB_CRL &= ~(GPIOB_MODE6_VAL(0b11) | GPIOB_MODE7_VAL(0b11) | GPIOB_CNF6_VAL(0b11) | GPIOB_CNF7_VAL(0b11)); // Clear PB6 and PB7 before configure these 2 pins. Page 171

  //11: Output mode, max speed 50 MHz. and 11: Alternate function output Open-drain: Page 171
  GPIOB_CRL |= (GPIOB_MODE6_VAL(0b11) | GPIOB_MODE7_VAL(0b11) | GPIOB_CNF6_VAL(0b11) | GPIOB_CNF7_VAL(0b11));

  // Disable i2c Peripheral
  I2C1_CR1 &= ~PE;

  //Reset I2C
  I2C1_CR1 |= SWRST;
  I2C1_CR1 &= ~SWRST;

//  I2C1_CR1 |= NOSTRETCH;

  // According to page 778 I2C_CCR = 180, thus 180 x 27,78ns ~ 5000ns to allow 100KHz SCL (Slow mode)
  I2C1_CCR = 180;

  // According to page 782 I2C1_TRISE = 37 (36 + 1), for maximum allowed at fPCLK1 = 36MHz
  // Note: TRISE[5:0] = 2 in reset value
  I2C1_TRISE = PCLK1_FREQ_IN_MHZ + 1;

  //26.6.2 I2C Control register 2 (I2C_CR2) page 774
  I2C1_CR2 = PCLK1_FREQ_IN_MHZ;
  //I2C1_CR2 = ITBUFEN|ITEVTEN|ITERREN|PCLK1_FREQ_IN_MHZ; // TODO improve event handlers
  //I2C1_CR2 = ITEVTEN|ITERREN|PCLK1_FREQ_IN_MHZ;
/*
  //TODO Enable interrupts when implement event handlers
  __nvic_set_priority(I2C1_EV_IRQn, I2C1_EV_PRIO);
  __nvic_enable_irq(I2C1_EV_IRQn);
  __nvic_set_priority(I2C1_ER_IRQn, I2C1_ER_PRIO);
  __nvic_enable_irq(I2C1_ER_IRQn);
*/
  I2C1_CR1 |= PE;

}
//    I2C1_SR1 &= ~AF;
#define CHECK_I2C_NACK_OR_TIMEOUT(fn, errorCode) \
  if (I2C1_SR1 & AF) {\
    err = errorCode##_NACK;\
    goto fn##_finish;\
  }\
\
  if (timing <= milliseconds()) {\
    err = errorCode##_TIMEOUT;\
    goto fn##_finish;\
  }

//https://electronics.stackexchange.com/questions/272427/stm32-busy-flag-is-set-after-i2c-initialization
//https://www.st.com/content/ccc/resource/technical/document/errata_sheet/7d/02/75/64/17/fc/4d/fd/CD00190234.pdf/files/CD00190234.pdf/jcr:content/translations/en.CD00190234.pdf#page26
//https://electronics.stackexchange.com/questions/267972/i2c-busy-flag-strange-behaviour
//760 Page master transmitting
//https://github.com/afiskon/stm32-ssd1306

enum i2c1_err_e hal_i2c1_write(uint8_t dev_address, uint8_t mem_address, uint8_t *data, uint16_t data_size, uint64_t timeout)
{

  if (data_size == 0 || data == NULL)
    return I2C1_SUCCESS;

  enum i2c1_err_e err = I2C1_SUCCESS;
  uint64_t timing = milliseconds() + timeout;

  // Check if bus is busy
  while ((I2C1_SR2 & BUSY) && (timing > milliseconds()));

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_BUSY_BUS)

  I2C1_SR1 &= ~AF; // Clear ACK Failure bit
  I2C1_SR1 &= ~(POS); // Clear POS

  // Generating start
  I2C1_CR1 |= START;
  while ((!(I2C1_SR1 & SB)) && (timing > milliseconds()));

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_START_BUS)

  // Begin address send
  (void)I2C1_SR1;  // Clear bit SB
  I2C1_DR = (dev_address << 1);  // Write bit (R/W = 0)

  while ((!(I2C1_SR1 & ADDR)) && (timing > milliseconds()));
  //This bit is cleared by software reading SR1 register followed reading SR2, or by hardware when PE=0. page: 780
  (void)I2C1_SR1;
  (void)I2C1_SR2;

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_ADDRESS_BUS)

  I2C1_DR = (uint8_t)mem_address; // Add memory address
  //while ((!(I2C1_SR1 & TxE)) && (timing > milliseconds())); // Waiting for ACK or timeout

  //CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_MEMORY_ADDRESS_BUS)

  while (data_size > 0) {
    while ((!(I2C1_SR1 & TxE)) && (timing > milliseconds())); // Waiting for ACK or timeout

    CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_MEMORY_ADDRESS_BUS)

    I2C1_DR = (uint8_t)*data;

    --data_size;
    ++data;

    if ((I2C1_SR1 & BTF) && (data_size > 0)) {
      I2C1_DR = (uint8_t)*data;

      --data_size;
      ++data;
    }

    while ((!(I2C1_SR1 & BTF)) && (timing > milliseconds()));

    CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_STOP_BUS);
  }

hal_i2c1_write_finish:

  if (err != I2C1_SUCCESS) {
    if (I2C1_SR1 & AF) {
      I2C1_SR1 &= ~AF;
      I2C1_CR1 |= STOP;
    }
    return err;
  }

  I2C1_CR1 |= STOP; //// Generating stop

  return I2C1_SUCCESS;
}

/*
void I2C1_EV_IRQHandler()
{
  uint16_t sr = I2C1_SR1;

  if (sr & SB) {
    //EV5
    I2C1_DR = i2c1_addr;
  } else if (sr & ADDR) {
    //EV6
    I2C1_DR = i2c1_mem_address;
  } else if (i2c1_data_size > 0) {
    //EV8_1
    if (sr & TxE) {
       I2C1_DR = *(i2c1_data++);
      --i2c1_data_size;
    }
  } else if (sr & TxE) {
    //EV8_2
    if (sr & BTF)
      I2C1_CR1 |= STOP;
  }
}


void I2C1_ER_IRQHandler()
{
  i2c1_error = I2C1_SR1;

  I2C1_CR1 |= STOP;
//TODO implement error handlers callback here
}
*/
