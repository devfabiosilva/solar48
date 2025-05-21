#include <gpios.h>
#include <time.h>
//#include <stdlib.h>

void setup()
{
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

