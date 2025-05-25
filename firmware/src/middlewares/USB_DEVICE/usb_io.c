//#include <registers.h>
#include <stdio.h>
#include <usb_device.h>
#include <gpios.h>
//TODO enable RCC->APB1ENR |= RCC_APB1ENR_USBEN;
/*
sudo usermod -aG dialout $USER
minicom -D /dev/ttyACM0 -b 115200
screen /dev/ttyACM0 115200
cat /dev/ttyACM0
watch -n 0.5 "ls /dev/ttyACM*"
*/
extern char _end;
extern char _estack;
extern char _sheap;
extern char _eheap;

void usb_print_memory_usage(void)
{

    char buffer[200];
    uint32_t heap_used = (uint32_t)&_eheap - (uint32_t)&_sheap;
    uint32_t total_ram = (uint32_t)&_estack - (uint32_t)&_end;

    int len = snprintf(buffer, sizeof(buffer),
        "RAM: heap used %lu bytes, total RAM: %lu bytes\r\n",
        heap_used, total_ram);

    CDC_Transmit_FS((uint8_t*)buffer, len);
   //CDC_Transmit_FS((uint8_t*)"test", sizeof("test")-1);
}

