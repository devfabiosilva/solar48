#ifndef ERRORS_H
  #define ERRORS_h

typedef void (*error_callback_t)(int);
void error_handler(int error);

#define ERROR_CALLBACK(fn, error) \
  if (fn)\
    fn(error); \
  else \
    error_handler(error);

// USB INITIALIZATION ERROR
#define E_USB_INIT 100
#define E_USB_REGISTER_CLASS 101
#define E_USB_REGISTER_INTERFACE 102
#define E_USB_START 103


#define E_NOT_IMPLEMENTED -1

#endif

