// References: https://epics-modbus.readthedocs.io/en/latest/overview.html

#include <rs485.h>
#include <hal_uart.h>
#include <system.h>
#include <time.h>
#include <solar48_config.h>
#include <crc.h>
#include <stdlib.h>
#include <string.h>

static uint8_t modbus_master_buffer[256];
static uint8_t modbus_slave_buffer[256];

static SOLAR48_RS485_RTU master_rs485_rtu = {0};
static SOLAR48_RS485_RTU slave_rs485_rtu = {0};

extern void app_panic(const char *);

static inline uint16_t swap16(uint16_t x) {
    uint32_t y = (uint32_t)x;
    __asm__("rev16 %0, %0" : "+r"(y));
    return (uint16_t)y;
}

static void swap16_array_fast(uint16_t *arr, size_t len)
{

// rev16 instructions swaps 2 half words, so calling this is faster than simple swap bytes using C.
// Example: a = 0x01020304
// mov R0, #0x01020304
// rev16 R0, R0
// RESULT: 0x02010403
// Refs.: https://developer.arm.com/documentation/ddi0602/2025-06/Base-Instructions/REV16--Reverse-bytes-in-16-bit-halfwords-
//        https://developer.arm.com/documentation/dui0379/e/arm-and-thumb-instructions/rev16

  uint32_t *p = (uint32_t *)arr;
  size_t n = len >> 1; // Divide by 2

  while (n > 0) {
    __asm__("rev16 %0, %0": "=r"(*p): "0"(*p));
    p++;
    --n;
  }

  // If even
  if (len & 1)
    arr[len - 1] = swap16(arr[len - 1]);

}

static int master_prepare_to_send(uint8_t *out_data_size, uint8_t slave_address, MB_FUNCION function, uint16_t mem_address, uint16_t n)
{

  if (slave_address > 247)
    return E_MASTER_INVALID_SLAVE_ADDRESS;

  modbus_master_buffer[0] = slave_address;
  PDU_FRAME *frame = (PDU_FRAME *)&modbus_master_buffer[1];

  uint16_t m;
  int error_on_invalid_n;

  switch (function) {
    case READ_COILS:
      error_on_invalid_n = E_MASTER_INVALID_NUMBER_OF_COILS;

      goto master_prepare_to_send_read_discrete;

    case READ_DISCRETE_INPUTS:
      error_on_invalid_n = E_MASTER_INVALID_NUMBER_OF_DISCRETE_INPUTS;

      goto master_prepare_to_send_read_discrete;

    case READ_HOLDING_REGISTERS:
      error_on_invalid_n = E_MASTER_INVALID_NUMBER_OF_HOLDING_REGISTERS;

      goto master_prepare_to_send_read_register;

    case READ_INPUT_REGISTERS:
      error_on_invalid_n = E_MASTER_INVALID_NUMBER_OF_INPUT_REGISTERS;

      goto master_prepare_to_send_read_register;
  }

  return E_UNEXPECTED_FUNCTION; // Guard

master_prepare_to_send_read_register:

  if (n < 1 || n > 125)
    return error_on_invalid_n;

  frame->pdu_read_req.function_code = function;
  frame->pdu_read_req.starting_address = swap16(mem_address); // Big endian
  frame->pdu_read_req.number_of_registers = swap16(n); // Big endian

  #define READ_REGISTER_REQ_SIZE (1 + sizeof(struct pdu_read_req_t))
  *((uint16_t *)&modbus_master_buffer[READ_REGISTER_REQ_SIZE]) = crc16(modbus_master_buffer, READ_REGISTER_REQ_SIZE);
  *out_data_size = (READ_REGISTER_REQ_SIZE + 2);
  #undef READ_REGISTER_REQ_SIZE

  return 0;

master_prepare_to_send_read_discrete:

  if (n < 1 || n > 2000)
    return error_on_invalid_n; // 6.1 01 (0x01) Read Coils - Page 12, 6.2 02 (0x02) Read Discrete Inputs

  m = n >> 3; // Divide by 8
  if (n & 7) // Remainder
    ++m;

  frame->pdu_read_discrete_req.function_code = function;
  frame->pdu_read_discrete_req.starting_address = swap16(mem_address); // Big endian
  frame->pdu_read_discrete_req.number_of_discrete = swap16(m); // Big endian

  #define READ_DISCRETE_REQ_SIZE (1 + sizeof(struct pdu_read_discrete_req_t))
  *((uint16_t *)&modbus_master_buffer[READ_DISCRETE_REQ_SIZE]) = crc16(modbus_master_buffer, READ_DISCRETE_REQ_SIZE);
  *out_data_size = (READ_DISCRETE_REQ_SIZE + 2);
  #undef READ_DISCRETE_REQ_SIZE

  return 0;
}

#define MASTER_TRANSFER_SUCCESS 0
#define MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE 1

#define RS485_ERROR_CODE(x) (x + 0x80)
// Helper function. All data is read from master an checks for validation
static int master_check_receive(MB_FUNCION *function, uint8_t **data, uint16_t *data_size)
{
  *data = NULL;
  *data_size = 0;
  *function = master_rs485_rtu.function;

  if (master_rs485_rtu.address != modbus_master_buffer[0])
    return E_MASTER_INVALID_RECEIVE_SLAVE_ADDRESS;

  bool resize_vec_size = false;
  uint16_t crc;
  uint8_t 
    u8_sz,
    *ptr,
    error_code;

  int 
    ret_code = MASTER_TRANSFER_SUCCESS,
    error_on_catch_exception,
    error_unexpected_function,
    error_invalid_data_size,
    error_invalid_crc;

  PDU_FRAME *frame = (PDU_FRAME *)&modbus_master_buffer[1];

  switch (master_rs485_rtu.function) {
    case READ_COILS:
      error_on_catch_exception = E_UNEXPECTED_READ_COILS_ERROR_CODE;
      error_unexpected_function = E_UNEXPECTED_READ_COILS_FUNCTION;
      error_invalid_data_size = E_INVALID_READ_COILS_DATA_SIZE;
      error_invalid_crc = E_INVALID_READ_COILS_CRC;

      goto master_check_receive_read_discrete;

    case READ_DISCRETE_INPUTS:
      error_on_catch_exception = E_UNEXPECTED_READ_DISCRETE_INPUTS_ERROR_CODE;
      error_unexpected_function = E_UNEXPECTED_DISCRETE_INPUTS_FUNCTION;
      error_invalid_data_size = E_INVALID_DISCRETE_INPUTS_DATA_SIZE;
      error_invalid_crc = E_INVALID_DISCRETE_INPUTS_CRC;

      goto master_check_receive_read_discrete;

    case READ_HOLDING_REGISTERS:
      error_on_catch_exception = E_UNEXPECTED_READ_HOLDING_REGISTERS_ERROR_CODE;
      error_unexpected_function = E_UNEXPECTED_READ_HOLDING_REGISTERS_FUNCTION;
      error_invalid_data_size = E_INVALID_READ_HOLDING_REGISTERS_DATA_SIZE;
      error_invalid_crc = E_INVALID_READ_HOLDING_REGISTERS_CRC;

      goto master_check_receive_read_register;

    case READ_INPUT_REGISTERS:
      error_on_catch_exception = E_UNEXPECTED_READ_INPUT_REGISTERS_ERROR_CODE;
      error_unexpected_function = E_UNEXPECTED_READ_INPUT_REGISTERS_FUNCTION;
      error_invalid_data_size = E_INVALID_READ_INPUT_REGISTERS_DATA_SIZE;
      error_invalid_crc = E_INVALID_READ_INPUT_REGISTERS_CRC;

      goto master_check_receive_read_register;

  }

  return E_UNEXPECTED_RECEIVE_FUNCTION; // Guard. Never gets here

master_check_receive_read_register:
  if ((frame->pdu_read_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    error_code = frame->pdu_read_error_exception.error_or_exception_code;
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_read_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_read_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1; // master_check_receive_copy_buffer1 -> Size is the same as number of byte
    }

    return error_on_catch_exception;
  }

  if (frame->pdu_read_resp.function_code != master_rs485_rtu.function)
    return error_unexpected_function;

  u8_sz = frame->pdu_read_resp.byte_count;
  // (2*125 -> See: 6.3 03 (0x03) Read Holding Registers - page 15) + sizeof(address) + sizeof(function) + sizeof(count) + sizeof(crc) = 255
  if (u8_sz < 1 || u8_sz > 250)
    return error_invalid_data_size;

  // Before copy crc, we need to divide u8_sz / 2. Status type is uint16_t (2 bytes. See page 15)
  memcpy(&crc, &(((uint8_t *)&frame->pdu_read_resp.status)[u8_sz]), sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, (size_t)(u8_sz + offsetof(struct pdu_read_resp_t, status) + 1)) == crc) {
    ptr = (uint8_t *)&frame->pdu_read_resp.status[0];
    resize_vec_size = true; // master_check_receive_copy_buffer2 -> Vector is 2 bytes long
    goto master_check_receive_copy_buffer1;
  }

  return error_invalid_crc;

master_check_receive_read_discrete:
  if ((frame->pdu_read_discrete_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    error_code = frame->pdu_read_discrete_error_exception.error_or_exception_code;
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_read_discrete_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_read_discrete_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1;
    }

    return error_on_catch_exception;
  }

  if (frame->pdu_read_discrete_resp.function_code != master_rs485_rtu.function)
    return error_unexpected_function;
        
  u8_sz = frame->pdu_read_discrete_resp.byte_count;
  // (250 + 1 -> See: 6.1 01 (0x01) Read Coils - page 12) + sizeof(address) + sizeof(function) + sizeof(count) + sizeof(crc) = 256
  if (u8_sz < 1 || u8_sz > 251)
    return error_invalid_data_size;

  memcpy(&crc, &frame->pdu_read_discrete_resp.status[u8_sz], sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, (size_t)(u8_sz + offsetof(struct pdu_read_discrete_resp_t, status) + 1)) == crc) {
    ptr = &frame->pdu_read_discrete_resp.status[0];
    goto master_check_receive_copy_buffer1;
  }

  return error_invalid_crc;

master_check_receive_copy_buffer1:
  if ((*data = (uint8_t *)malloc((size_t)u8_sz))) {
    memcpy((void *)(*data), (void *)ptr, (size_t)u8_sz);

    if (resize_vec_size) {
      *data_size = (uint16_t)(u8_sz >> 1);
      swap16_array_fast((uint16_t *)*data, (size_t)*data_size);
    } else
      *data_size = (uint16_t)u8_sz;

    return ret_code;
  }

  app_panic("mds:ckrcv1");
  return -1; // Never gets here
}

#undef RS485_ERROR_CODE

static void _master_receive(int status)
{
  uint8_t *data = NULL;
  uint16_t data_size = 0;
  MB_FUNCION func;
  switch (status) {
    case UART1_RECEIVE_COMPLETE:
      status = master_check_receive(&func, &data, &data_size);

      // STATUS = 0 => receive is valid and data is available on buffer
      if ((status == MASTER_TRANSFER_SUCCESS) || (status == MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE)) {
        // From here data is valid and formatted to Little endian in 16 bits results
        sys_unlock(&master_rs485_rtu.lock);
        master_rs485_rtu.callback(status, func, data, data_size);

        if (data) // Guard
          free(data);
        else
          app_panic("_mrcv:mem1");

        return;
      }

    default:
      if (data)
        app_panic("_mrcv:mem2");
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status, MB_FUNCION_UNDEFINED, data, data_size);
  }
}

#undef MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE
#undef MASTER_TRANSFER_SUCCESS

static void _master_send_req(int status)
{
  int err;
  switch (status) {
    case UART1_TRANSFER_COMPLETE:
      if (!(err = uart1_receive(modbus_master_buffer, sizeof(modbus_master_buffer), _master_receive, master_rs485_rtu.timeout_ms)))
        return;

    default:
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status, MB_FUNCION_UNDEFINED, NULL, 0);
  }
}

// Implement RTU only (uses UART1)
int master_send_req(uint8_t slave_address, MB_FUNCION function, uint16_t mem_address, uint16_t n, uint32_t timeout_ms, mb_master_recv_callback response_callback)
{

  TIMEOUT_MS timeout;

  if (!sys_try_lock(&master_rs485_rtu.lock, &timeout, MASTER_RS485_TIMEOUT_MS, NULL))
    return E_MASTER_MODBUS_BUSY;

  master_rs485_rtu.address = slave_address;
  master_rs485_rtu.function = function;
  master_rs485_rtu.timeout_ms = timeout_ms;
  master_rs485_rtu.callback = response_callback;

  uint8_t out_data_size;
  int err = master_prepare_to_send(&out_data_size, slave_address, function, mem_address, n);

  if (err) {
    sys_unlock(&master_rs485_rtu.lock);
    return err;
  }

  if ((err = uart1_transmit(modbus_master_buffer, (size_t)out_data_size, _master_send_req, timeout_ms)))
    sys_unlock(&master_rs485_rtu.lock);

  return err;
}
//__builtin_popcount()
