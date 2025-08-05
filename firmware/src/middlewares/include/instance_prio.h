#ifndef INSTANCE_PRIO_H
 #define INSTANCE_PRIO_H

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#define PRIO_4 (configMAX_PRIORITIES - 1) // MAX PRIORITY GOES HERE
#define PRIO_3 (PRIO_4 - 1)
#define PRIO_2 (PRIO_3 - 1)
#define PRIO_1 (PRIO_2 - 1)
#define PRIO_0 (PRIO_1 - 1)

_Static_assert(PRIO_0 > -1, "PRIO values should not have negative values");

#endif

