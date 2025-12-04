//Inter-integrated circuit (I2C) interface page 752
#include <stdint.h>
#include <stddef.h>
#include <registers.h>
#include <sys_interrupts.h>
#include <hal_i2c.h>
#include <stdbool.h>
#include <solar48_config.h>
#include <time.h>

volatile bool i2c1_lock;

void hal_i2c1_init()
{

  //RCC_APB2ENR |= IOPBEN; // IO port B clock enable page 114
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // fPCLK1 = 36MHz or TPCLK1 = 27.78ns
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

  // According to page 778 I2C_CCR = 180, thus 180 x 27,78ns ~ 5000ns to allow 100KHz SCL (Slow mode)
  //I2C1_CCR = 180; //100kHz
  I2C1_CCR = FS|30; //400kHz

  // According to page 782 I2C1_TRISE = 37 (36 + 1), for maximum allowed at fPCLK1 = 36MHz
  // Note: TRISE[5:0] = 2 in reset value
  I2C1_TRISE = PCLK1_FREQ_IN_MHZ + 1;

  //26.6.2 I2C Control register 2 (I2C_CR2) page 774
  I2C1_CR2 = PCLK1_FREQ_IN_MHZ;

  I2C1_CR1 |= PE;

}

#define CHECK_I2C_NACK_OR_TIMEOUT(fn, errorCode) \
  if (I2C1_SR1 & AF) {\
    err = errorCode##_NACK;\
    goto fn##_finish;\
  }\
\
  if (is_timeout_ms(&timing)) {\
    err = errorCode##_TIMEOUT;\
    goto fn##_finish;\
  }

//760 Page master transmitting
enum i2c1_err_e hal_i2c1_write(uint8_t dev_address, uint8_t mem_address, uint8_t *data, uint16_t data_size, uint32_t timeout)
{

  if (data_size == 0 || data == NULL)
    return I2C1_SUCCESS;

  if (i2c1_lock)
    return I2C1_PORT_BUSY;

  i2c1_lock = true;

  enum i2c1_err_e err = I2C1_SUCCESS;
  //uint64_t timing = milliseconds() + timeout;
  TIMEOUT_MS timing;

  init_timeout_ms(&timing, timeout);

  // Check if bus is busy
  while ((I2C1_SR2 & BUSY) && (!is_timeout_ms(&timing)));

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_BUSY_BUS)

  I2C1_SR1 &= ~AF; // Clear ACK Failure bit
  I2C1_SR1 &= ~(POS); // Clear POS

  // Generating start
  I2C1_CR1 |= START;
  while ((!(I2C1_SR1 & SB)) && (!is_timeout_ms(&timing)));

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_START_BUS)

  // Begin address send
  (void)I2C1_SR1;  // Clear bit SB
  I2C1_DR = (dev_address << 1);  // Write bit (R/W = 0)

  while ((!(I2C1_SR1 & ADDR)) && (!is_timeout_ms(&timing)));
  //This bit is cleared by software reading SR1 register followed reading SR2, or by hardware when PE=0. page: 780
  (void)I2C1_SR1;
  (void)I2C1_SR2;

  CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_ADDRESS_BUS)

  I2C1_DR = (uint8_t)mem_address; // Add memory address
  //while ((!(I2C1_SR1 & TxE)) && (timing > milliseconds())); // Waiting for ACK or timeout

  //CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_MEMORY_ADDRESS_BUS)

  while (data_size > 0) {
    while ((!(I2C1_SR1 & TxE)) && (!is_timeout_ms(&timing))); // Waiting for ACK or timeout

    CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_MEMORY_ADDRESS_BUS)

    I2C1_DR = (uint8_t)*data;

    --data_size;
    ++data;

    if ((I2C1_SR1 & BTF) && (data_size > 0)) {
      I2C1_DR = (uint8_t)*data;

      --data_size;
      ++data;
    }

    while ((!(I2C1_SR1 & BTF)) && (!is_timeout_ms(&timing)));

    CHECK_I2C_NACK_OR_TIMEOUT(hal_i2c1_write, I2C1_ERR_DEV_STOP_BUS);
  }

hal_i2c1_write_finish:

  if (err != I2C1_SUCCESS) {
    if (I2C1_SR1 & AF) {
      I2C1_SR1 &= ~AF;
      I2C1_CR1 |= STOP;
    }

    i2c1_lock = false;

    return err;
  }

  I2C1_CR1 |= STOP; //// Generating stop

  i2c1_lock = false;

  return I2C1_SUCCESS;
}

