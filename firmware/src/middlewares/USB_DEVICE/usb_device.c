/*
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
*/
#include <errors.h>
#include <stddef.h>

//USBD_HandleTypeDef hUsbDeviceFS;

error_callback_t usb_err_fn = NULL;

#define USB_ERROR(error) ERROR_CALLBACK(usb_err_fn, error)

void init_usb_device(error_callback_t err_cb)
{
  USB_ERROR(E_NOT_IMPLEMENTED)
/*
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
    USB_ERROR(USB_INIT);
    return;
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
    USB_ERROR(USB_REGISTER_CLASS);
    return;
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    USB_ERROR(USB_REGISTER_INTERFACE);
    return;
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    USB_ERROR(USB_START);
    return;
  }
  */
}

#undef USB_ERROR

