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

// USB TRANSMIT ERROR
#define E_USB_TRANSMIT_BUSY 120
#define E_USB_TRANSMIT_FAIL 121

// USB HAL CALBACK ERROR
#define E_USB_HAL_PCD_HS 130

// USB HAL RECEIVE ERROR
#define E_USB_RECEIVE_ERROR 140

// USB HAL LOCK ERROR
#define E_USB_LOCK_ERROR 150

// USB HAL RECEIVE PROCESS BUSY
#define E_USB_RECEIVE_PROC_BUSY 160

// USB BYTE CHUNK SENDING
#define E_USB_SEND_CHUNK_INIT_BUSY 170
#define E_USB_SEND_CHUNK_SEND_FAIL 171
#define E_USB_SEND_CHUNK_BUSY 172


// UART1 ERROR SENDING
#define E_UART1_BUSY 180
#define E_UART1_LOCKED 181
#define E_UART1_TIMEOUT 182
#define E_UART1_DMA1_CH4_TRANSMIT_ERROR 183

///////////////////////////////////

// ERROR IN PROCESS
#define PROCESS_BUSY 90

#define E_NOT_IMPLEMENTED -1

#endif

