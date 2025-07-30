#ifndef USB_IO_H
  #define USB_IO_H

int usb_printf(const char *, ...);
uint32_t rtc_get_timestamp();
void usb_print_memory_info(void);

#endif

