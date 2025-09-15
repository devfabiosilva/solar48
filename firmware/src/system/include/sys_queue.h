#ifndef SYS_QUEUE_H
 #define SYS_QUEUE_H

#include <stdbool.h>

typedef int (*sys_queue_cb)(void *);

typedef struct sys_queue_vec_t {
  volatile void *ctx;
  volatile sys_queue_cb function;
  volatile sys_queue_cb destroyer;
} SYS_QUEUE_VEC;

typedef enum queue_flag_e {
  QUEUE_RUNNING = 0,
  QUEUE_SEND_DEINIT_SIGNAL,
  QUEUE_READY_DEINIT
} QUEUE_FLAG;

typedef struct sys_queue_t {
  volatile QUEUE_FLAG flag;
  volatile bool lock;
  volatile bool enable_global_error;
  volatile int queue_size;
  volatile int head;
  volatile int tail;
  volatile int count;
  volatile SYS_QUEUE_VEC *vector;
} SYS_QUEUE;

void sys_queue_init(SYS_QUEUE **, int, bool);
void sys_queue_deinit(SYS_QUEUE **);
void run_queue_run(SYS_QUEUE *);
bool sys_queue(SYS_QUEUE *, sys_queue_cb, void *, sys_queue_cb);

#endif
