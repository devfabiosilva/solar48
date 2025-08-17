#ifndef USB_DEVICE_H
  #define USB_DEVICE_H

#include <errors.h>
#include <hal_usb.h>

void *USBD_static_malloc(uint32_t size);
#define USBD_malloc         (uint32_t *)USBD_static_malloc
USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);

int cdc_transmit_is_busy();

void init_usb();
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

#endif
