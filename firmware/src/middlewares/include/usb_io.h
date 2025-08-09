#ifndef USB_IO_H
  #define USB_IO_H

#include <stddef.h>

int usb_printf(const char *, ...);
uint32_t rtc_get_timestamp();
void usb_print_memory_info(void);
int usb_send_chunk(uint8_t **, size_t *);

#endif

