#include <sys_queue.h>
#include <solar48_config.h>
#include <errors.h>
#include <time.h>
#include <watchdog.h>
#include <stddef.h>
#include <stdlib.h>
#include <system.h>

#define QUEUE_ERROR(x) \
if (queue->enable_global_error) error_handler(x);

extern void app_panic(const char *);

void sys_queue_init(SYS_QUEUE **queue, int queue_size, bool enable_global_error)
{
  if (*queue) {
    app_panic("queue:INIT");
    return; // Never reaches here
  }

  if (!((*queue) = (SYS_QUEUE *)malloc(sizeof(SYS_QUEUE)))) {
    app_panic("queue:MEM");
    return; // Never reaches here
  }

  (*queue)->flag = QUEUE_RUNNING;
  (*queue)->lock = false;
  (*queue)->enable_global_error = enable_global_error;

  if (queue_size > 0)
    (*queue)->queue_size = queue_size;
  else
    (*queue)->queue_size = DEFAULT_SYS_QUEUE_SIZE;

  (*queue)->head = 0;
  (*queue)->tail = 0;
  (*queue)->count = 0;

  if (!((*queue)->vector = (SYS_QUEUE_VEC *)malloc((*queue)->queue_size*sizeof(SYS_QUEUE_VEC)))) {
    free((void *)(*queue));
    app_panic("queuevec:MEM");
    return; // Never reaches here
  }

}

void sys_queue_deinit(SYS_QUEUE **queue)
{
  if (*queue) {

    TIMEOUT_MS timeout;
    sys_try_lock(&((*queue)->lock), &timeout, SYS_QUEUE_DEINIT_TIMEOUT_MS, "queue:DEINIT1");

    (*queue)->flag = QUEUE_SEND_DEINIT_SIGNAL;

    sys_unlock(&(*queue)->lock);

    while (__atomic_load_n(&((*queue)->flag), __ATOMIC_SEQ_CST) != QUEUE_READY_DEINIT)
      if (is_timeout_ms(&timeout)) {
        app_panic("queue:DEINIT2");
        return; // Never reaches here
      }

    sys_try_lock(&(*queue)->lock, &timeout, SYS_QUEUE_DEINIT_TIMEOUT_MS, "queue:DEINIT2");

    free((void *)(*queue)->vector);
    (*queue)->vector = NULL;

    (*queue) = NULL;
  }
}

bool sys_queue(SYS_QUEUE *queue, sys_queue_cb function, void *ctx, sys_queue_cb destroyer)
{

  if (!queue)
    return false;

  TIMEOUT_MS timeout;
  if (!sys_try_lock(&(queue->lock), &timeout, SYS_QUEUE_TIMEOUT_MS, NULL)) {
    QUEUE_ERROR(E_SYS_ADD_QUEUE_TIMEOUT)
    return false;
  }

  if (queue->flag != QUEUE_RUNNING) {
    sys_unlock(&queue->lock);
    QUEUE_ERROR(E_SYS_ADD_QUEUE_NOT_RUNNING)
    return false;
  }

  if (queue->count >= queue->queue_size) {
    sys_unlock(&queue->lock);
    QUEUE_ERROR(E_SYS_QUEUE_FULL);
    return false;
  }

  int head = queue->head;
  SYS_QUEUE_VEC *item = (SYS_QUEUE_VEC *)&queue->vector[head];

  item->ctx = ctx;
  item->function = function;
  item->destroyer = destroyer;

  queue->head = (head + 1) % queue->queue_size;
  queue->count++;

  sys_unlock(&queue->lock);

  return true;
}

#define EXECUTE_QUEUE_ITEM(x) \
{ \
  err = item->x(ctx); \
  if ((err != 0) && queue->enable_global_error) \
    error_handler(err); \
\
  item->x = NULL; \
}

void run_queue_run(SYS_QUEUE *queue)
{

  if (!queue)
    return;

  TIMEOUT_MS timeout;
  if (!sys_try_lock(&queue->lock, &timeout, SYS_QUEUE_TRY_RUN_TIMEOUT_MS, NULL)) {
    QUEUE_ERROR(E_SYS_RUN_QUEUE_TIMEOUT)
    return;
  }

  int err;
  int tail;
  SYS_QUEUE_VEC *item;
  void *ctx;

  QUEUE_FLAG flag = __atomic_load_n(&queue->flag, __ATOMIC_SEQ_CST);

  if (flag == QUEUE_RUNNING) {

    if (queue->count < 1) {
      sys_unlock(&queue->lock);
      return;
    }

    tail = (int)queue->tail;
    item = (SYS_QUEUE_VEC *)&queue->vector[tail];
    ctx = (void *)item->ctx;

    if (item->function)
      EXECUTE_QUEUE_ITEM(function)
    else {
      app_panic("queuefun:NULL");
      return; // Never reaches here
    }

    if (item->destroyer)
      EXECUTE_QUEUE_ITEM(destroyer)

    queue->tail = (tail + 1) % queue->queue_size;
    queue->count--;

  } else if (flag == QUEUE_SEND_DEINIT_SIGNAL) {
    init_timeout_ms(&timeout, SYS_QUEUE_DESTROY_TIMEOUT_MS);

    //After singnal to teminate queue process, execute all left destroy contexts
    while (queue->count > 0) {
      if (is_timeout_ms(&timeout)) {
        app_panic("queuedest:NULL");
        return; // Never reaches here
      }

      tail = (int)queue->tail;
      item = (SYS_QUEUE_VEC *)&queue->vector[tail];
      ctx = (void *)item->ctx;

      if (item->destroyer)
        EXECUTE_QUEUE_ITEM(destroyer)

      queue->tail = (tail + 1) % queue->queue_size;
      queue->count--;
    }

   // Now queue is ready to be terminated with no leaks
    __atomic_store_n(&queue->flag, QUEUE_READY_DEINIT, __ATOMIC_RELEASE);
  }

  sys_unlock(&queue->lock);
}

