#include <process.h>
#include <stddef.h>

static volatile process_caller proc = NULL;
static volatile void *ctx_ptr = NULL;
static volatile int ret_code = 0;

// Add process to queue
bool add_process(process_caller caller, void *ctx)
{
  if (proc)
    return false;

  ctx_ptr = ctx;
  proc = caller;

  return true;
}

// run process
void run_process()
{
  if (proc) {
    void *ctx = (void *)ctx_ptr;
    ret_code = proc(ctx);
  }

  proc = NULL;
  ctx_ptr = NULL;
}

// If executed, returns last return code and clean it, NULL if process is not executed yet
int *get_ret_process(int *ret)
{
  if (proc)
    return NULL;

  *ret = ret_code;
  ret_code = 0;

  return ret;
}

