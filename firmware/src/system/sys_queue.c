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
// Init and reinit queue
void sys_queue_init(SYS_QUEUE *queue, int queue_size, bool enable_global_error)
{

  TIMEOUT_MS timeout;
  if (!sys_try_lock_if_gbl_is_false(&queue->global_lock, &queue->lock, &timeout, SYS_QUEUE_INIT_TIMEOUT_MS, "queue:INIT1")) {
    // Panic on global_lock always true: "queue:INIT2"
    // Panic on unlock timeout: "queue:INIT1"
    app_panic("queue:INIT2");
    return;
  }

  // lock = true at this point
  sys_try_lock(&queue->global_lock, &timeout, SYS_QUEUE_TRY_UNLOCK_GLOBAL_TIMEOUT_MS, "queue:INIT3");
  // lock_global is true at this point or PANIC: "queue:INIT3" if timeout

  // If flag = QUEUE_OFF or  lock | global_lock => any process is not accessing concurrently variables below after this point

  if (queue->vector) {
    app_panic("queue:INIT4"); // Guard: queue->vector must be NULL
    return; // It never reaches here
  }

  if (queue->flag != QUEUE_OFF) {
    app_panic("queue:INIT5");
    return;
  }

  queue->enable_global_error = enable_global_error;
  queue->queue_size = (queue_size > 0)?queue_size:DEFAULT_SYS_QUEUE_SIZE;
  queue->head = 0;
  queue->tail = 0;
  queue->count = 0;

  if (!(queue->vector = (SYS_QUEUE_VEC *)malloc(queue->queue_size*sizeof(SYS_QUEUE_VEC)))) {
    app_panic("queuevec:MEM");
    return; // Never reaches here
  }

  sys_unlock(&queue->lock);
  sys_unlock(&queue->global_lock);

  // Now queue is ready to run. We must use __atomic here
  __atomic_store_n(&queue->flag, QUEUE_RUNNING, __ATOMIC_RELEASE);

}

void sys_queue_deinit(SYS_QUEUE *queue)
{
  TIMEOUT_MS timeout;
  sys_try_lock(&queue->global_lock, &timeout, SYS_QUEUE_TRY_UNLOCK_GLOBAL_TIMEOUT_MS, "queue:DEINIT1");
  sys_try_lock(&queue->lock, &timeout, SYS_QUEUE_DEINIT_TIMEOUT_MS, "queue:DEINIT2");

  QUEUE_FLAG flag = (QUEUE_FLAG)queue->flag;

  if (flag == QUEUE_RUNNING)
    queue->flag = QUEUE_SEND_DEINIT_SIGNAL;
  else if (flag == QUEUE_OFF) {

    if (queue->vector) {
      app_panic("queue:DEINIT3");
      return; // Never reaches here
    }

    sys_unlock(&queue->lock);
    sys_unlock(&queue->global_lock);

    return;
  } else {
    app_panic("queue:DEINIT4");
    return; // Never reaches here
  }

  sys_unlock(&queue->lock);

  while (__atomic_load_n(&queue->flag, __ATOMIC_SEQ_CST) != QUEUE_READY_DEINIT)
    if (is_timeout_ms(&timeout)) {
      app_panic("queue:DEINIT5");
      return; // Never reaches here
    }

  sys_try_lock(&queue->lock, &timeout, SYS_QUEUE_DEINIT_TIMEOUT_MS, "queue:DEINIT6");

  // Will be OFF It guarantees that vector will never be used. After unlock global and lock local
  queue->flag = QUEUE_OFF;

  free((void *)queue->vector);
  queue->vector = NULL;

  sys_unlock(&queue->lock);
  sys_unlock(&queue->global_lock);
}

// *queue is NON NULL
int sys_queue(SYS_QUEUE *queue, sys_queue_cb function, void *ctx, sys_queue_cb destroyer)
{

  if (__atomic_load_n(&queue->flag, __ATOMIC_SEQ_CST) == QUEUE_OFF) {
    QUEUE_ERROR(E_SYS_QUEUE_QUEUE_OFF)
    return E_SYS_QUEUE_QUEUE_OFF;
  }

  TIMEOUT_MS timeout;
  if (!sys_try_lock_if_gbl_is_false(&queue->global_lock, &queue->lock, &timeout, SYS_QUEUE_TIMEOUT_MS, NULL)) {
    QUEUE_ERROR(E_SYS_ADD_QUEUE_TIMEOUT_OR_GLOBAL_LOCK)
    return E_SYS_ADD_QUEUE_TIMEOUT_OR_GLOBAL_LOCK;
  }

  // We don't need atomic here. Lock guarantees that only this procedure is accessing this flag
  if (queue->flag != QUEUE_RUNNING) {
    sys_unlock(&queue->lock);
    QUEUE_ERROR(E_SYS_ADD_QUEUE_NOT_RUNNING)
    return E_SYS_ADD_QUEUE_NOT_RUNNING;
  }

  if (queue->count >= queue->queue_size) {
    sys_unlock(&queue->lock);
    QUEUE_ERROR(E_SYS_QUEUE_FULL);
    return E_SYS_QUEUE_FULL;
  }

  int head = queue->head;
  SYS_QUEUE_VEC *item = (SYS_QUEUE_VEC *)&queue->vector[head];

  item->ctx = ctx;
  item->function = function;
  item->destroyer = destroyer;

  queue->head = (head + 1) % queue->queue_size;
  queue->count++;

  sys_unlock(&queue->lock);

  return 0;
}

#define EXECUTE_QUEUE_ITEM(x) \
{ \
  err = item->x(ctx); \
  if ((err != 0) && queue->enable_global_error) \
    error_handler(err); \
\
  item->x = NULL; \
}

// queue is NON NULL
void run_queue_run(SYS_QUEUE *queue)
{

  QUEUE_FLAG flag = __atomic_load_n(&queue->flag, __ATOMIC_SEQ_CST);

  if (flag == QUEUE_OFF || flag == QUEUE_READY_DEINIT)
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

  // We don't need atomic here. Lock guarantees that only this procedure is accessing this flag
  flag = queue->flag;

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
        app_panic("qdstry:FAIL");
        return; // Never reaches here
      }

      tail = (int)queue->tail;
      item = (SYS_QUEUE_VEC *)&queue->vector[(size_t)tail];
      ctx = (void *)item->ctx;

      if (item->destroyer)
        EXECUTE_QUEUE_ITEM(destroyer)

      queue->tail = (tail + 1) % queue->queue_size;
      queue->count--;
    }

   // Now queue is ready to be terminated with no leaks. We must use __atomic here
    __atomic_store_n(&queue->flag, QUEUE_READY_DEINIT, __ATOMIC_RELEASE);
  }

  sys_unlock(&queue->lock);
}

