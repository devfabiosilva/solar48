#ifndef PROCESS_H
 #define PROCESS_H
#include <stdbool.h>

typedef int (*process_caller)(void *);

void run_process();
bool add_process(process_caller, void *);
bool is_process_running(process_caller);
int *get_ret_process(int *);

void run_process_int_ext();
bool add_process_int_ext(process_caller, void *);
bool is_process_int_ext_running(process_caller);
int *get_ret_process_int_ext(int *);

void run_process_int_int();
bool add_process_int_int(process_caller, void *);
bool is_process_int_int_running(process_caller);
int *get_ret_process_int_int(int *);

#endif

