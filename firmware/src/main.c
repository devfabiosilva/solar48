#include <stddef.h>
#include <usb_device.h>
#include <gpios.h>
#include <time.h>
//#include <stdlib.h>

void setup()
{
  init_usb_device(NULL);
  init_gpios();
}

void run(void)
{
  delay_test();
}

void halt()
{
// It would not happen
}

