// TODO implement i2c bare metal ports peripheral
//Inter-integrated circuit (I2C) interface page 752

#include <stdint.h>
#include <stddef.h>
#include <registers.h>
#include <sys_interrupts.h>

#define PCLK1_FREQ_IN_MHZ 36

void hal_i2c1_init()
{

  RCC_APB2ENR |= IOPBEN; // IO port B clock enable page 114
  //TODO configure Port B output for I2C


  // fPCLK1 = 36MHz or TPCLK1 = 27.78ns
  RCC_APB1ENR |= I2C1EN; // I2C1 clock enable, page 116

  // According to page 778 I2C_CCR = 180, thus 180 x 27,78ns ~ 5000ns to allow 100KHz SCL (Slow mode)
  I2C1_CCR = 180;

  // According to page 782 I2C1_TRISE = 37 (36 + 1), for maximum allowed at fPCLK1 = 36MHz
  // Note: TRISE[5:0] = 2 in reset value
  I2C1_TRISE = PCLK1_FREQ_IN_MHZ + 1;

  //26.6.2 I2C Control register 2 (I2C_CR2) page 774
  I2C1_CR2 = ITBUFEN|ITEVTEN|ITERREN|PCLK1_FREQ_IN_MHZ;

  __nvic_set_priority(I2C1_EV_IRQn, I2C1_EV_PRIO);
  __nvic_enable_irq(I2C1_EV_IRQn);
  __nvic_set_priority(I2C1_ER_IRQn, I2C1_ER_PRIO);
  __nvic_enable_irq(I2C1_ER_IRQn);

  I2C_CR1 |= PE;
}


int hal_i2c1_write(uint16_t dev_address, uint16_t mem_address, uint16_t mem_address_size, uint8_t *data, uint16_t data_size, size_t timeout)
{

  // TODO implement
  return 0;
}


void I2C1_EV_IRQHandler()
{
//TODO implement event handlers callback here
}

void I2C1_ER_IRQHandler()
{
//TODO implement error handlers callback here
}

