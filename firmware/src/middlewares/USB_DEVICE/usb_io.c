#include <stdio.h>
#include <usb_device.h>
#include <memory.h>
/*
sudo usermod -aG dialout $USER
minicom -D /dev/ttyACM0 -b 115200
screen /dev/ttyACM0 115200
cat /dev/ttyACM0
watch -n 0.5 "ls /dev/ttyACM*"
*/

DETAILED_RAM dr;
DETAILED_FLASH df;

void usb_print_memory_usage(void)
{
  char buffer[256];

  get_flash_detailed(&df);
  get_ram_detailed(&dr);

  int len = snprintf(buffer, sizeof(buffer),
      "RAM:\n\tTOTAL SIZE = %lu, TOTAL AVAILABLE = %lu, STACK USED = %lu, STACK PEAK USED = %lu, PERCENT USED = %lu%%\r\nFLASH:\n\tTOTAL SIZE = %lu, USED: %lu, PERCENT USED: %lu%% \r\n",
      dr.size, dr.total_available, dr.stack_used, dr.stack_peak_used, dr.percent_used,
      df.size, df.used, df.percent_used);

  CDC_Transmit_FS((uint8_t*)buffer, len);
}

