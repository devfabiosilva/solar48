#ifndef USB_DEVICE_H
  #define USB_DEVICE_H
/*
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "usbd_def.h"
*/
#include <errors.h>
#include <hal.h>


void *USBD_static_malloc(uint32_t size);
#define USBD_malloc         (uint32_t *)USBD_static_malloc
USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);

void init_usb_device(error_callback_t);

#endif
