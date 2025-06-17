#ifndef PROCESS_H
 #define PROCESS_H
#include <stdbool.h>

typedef int (*process_caller)(void *);

void run_process();
bool add_process(process_caller, void *);

#endif 
