#ifndef USB_DEVICE_H
  #define USB_DEVICE_H

#include <errors.h>
#include <hal.h>

#define USBD_MAX_STR_DESC_SIZ     512
#define  USB_SIZ_STRING_SERIAL       0x1A
/** @defgroup USBD_DESC_Exported_Constants USBD_DESC_Exported_Constants
  * @brief Constants.
  * @{
  */
#define         DEVICE_ID1          (UID_BASE)
#define         DEVICE_ID2          (UID_BASE + 0x4)
#define         DEVICE_ID3          (UID_BASE + 0x8)

void *USBD_static_malloc(uint32_t size);
#define USBD_malloc         (uint32_t *)USBD_static_malloc
USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);

void init_usb_device(error_callback_t);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

#endif
