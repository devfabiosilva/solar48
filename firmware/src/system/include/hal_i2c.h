#ifndef HAL_I2C_H
  #define HAL_I2C_H

#include <stdint.h>
#include <stddef.h>

typedef struct hal_i2c_t {
  uint8_t address;
  //TODO implement;  
} HAL_I2C_TYPE;

int hal_i2c_write(HAL_I2C_TYPE *, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t, size_t);

#endif
