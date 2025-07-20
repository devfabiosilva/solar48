#ifndef HAL_I2C_H
  #define HAL_I2C_H

#include <stdint.h>
#include <stddef.h>

int hal_i2c1_write(uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t, size_t);

void hal_i2c1_init();

#endif
