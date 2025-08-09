#include <process.h>
#include <stddef.h>

#define PROCESS_DECLARE(name) \
static volatile process_caller _##name = NULL;\
static volatile void *ctx_ptr_##name = NULL;\
static volatile int ret_code_##name = 0;

// Add process to queue
#define ADD_PROCESS(name)\
bool add_##name(process_caller caller, void *ctx) \
{ \
  if (_##name) \
    return false; \
\
  ctx_ptr_##name = ctx; \
  _##name = caller; \
\
  return true; \
}

// run process
#define RUN_PROCESS(name)\
void run_##name() \
{ \
  if (_##name) { \
    void *ctx = (void *)ctx_ptr_##name; \
    ret_code_##name = _##name(ctx); \
    _##name = NULL; \
    ctx_ptr_##name = NULL; \
  } \
}

// If executed, returns last return code and clean it, NULL if process is not executed yet
#define GET_PROCESS(name) \
int *get_ret_##name(int *ret) \
{ \
  if (_##name) \
    return NULL; \
\
  *ret = ret_code_##name; \
  ret_code_##name = 0; \
\
  return ret; \
}

#define IS_PROCESS_RUNNING(name) \
inline bool is_##name##_running(process_caller caller) \
{ \
  return caller == _##name; \
}

PROCESS_DECLARE(process)
ADD_PROCESS(process)
RUN_PROCESS(process)
GET_PROCESS(process)
IS_PROCESS_RUNNING(process)

PROCESS_DECLARE(process_int_ext)
ADD_PROCESS(process_int_ext)
RUN_PROCESS(process_int_ext)
GET_PROCESS(process_int_ext)
IS_PROCESS_RUNNING(process_int_ext)

PROCESS_DECLARE(process_int_int)
ADD_PROCESS(process_int_int)
RUN_PROCESS(process_int_int)
GET_PROCESS(process_int_int)
IS_PROCESS_RUNNING(process_int_int)

