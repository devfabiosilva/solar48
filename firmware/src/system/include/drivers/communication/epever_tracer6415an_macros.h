#ifndef EPEVER_TRACER6415AN_MACROS_H
 #define EPEVER_TRACER6415AN_MACROS_H

#define FUNC_AS_JSON(func, text, error_prefix, json_txt, ...) \
char *func##_as_json(char *buf, size_t buf_sz, int *len) \
{ \
  text\
  \
  int len_or_error = snprintf(buf, buf_sz,  \
    json_txt, \
    __VA_ARGS__ \
  ); \
\
  if (len_or_error < 0) { \
    len_or_error = error_prefix##_JSON_UNABLE_TO_PARSE; \
    goto func##_as_json_error; \
  } \
\
  if ((size_t)len_or_error >= buf_sz) { \
    len_or_error = error_prefix##_JSON_BUF_OVERFLOW; \
\
func##_as_json_error: \
    error_handler(len_or_error); \
    len_or_error = 0; \
  } \
\
  buf[len_or_error] = 0; \
\
  if (len) \
    *len = len_or_error; \
\
  return buf; \
}

#endif