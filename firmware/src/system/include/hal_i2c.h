#ifndef HAL_I2C_H
  #define HAL_I2C_H

#include <stdint.h>
#include <stddef.h>

enum i2c1_err_e hal_i2c1_write(uint8_t, uint8_t, uint8_t *data, uint16_t, uint64_t);

void hal_i2c1_init();

enum i2c1_err_e {
  I2C1_SUCCESS = 0,
  I2C1_ERR_BUSY_BUS_TIMEOUT = 5,
  I2C1_ERR_BUSY_BUS_NACK,
  I2C1_ERR_START_BUS_TIMEOUT,
  I2C1_ERR_START_BUS_NACK,
  I2C1_ERR_DEV_ADDRESS_BUS_TIMEOUT,
  I2C1_ERR_DEV_ADDRESS_BUS_NACK,
  I2C1_ERR_DEV_MEMORY_ADDRESS_BUS_TIMEOUT,
  I2C1_ERR_DEV_MEMORY_ADDRESS_BUS_NACK,
  I2C1_ERR_DEV_STOP_BUS_TIMEOUT,
  I2C1_ERR_DEV_STOP_BUS_NACK
};

#endif
