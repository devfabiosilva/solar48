// References: https://epics-modbus.readthedocs.io/en/latest/overview.html

#include <rs485.h>
#include <hal_uart.h>
#include <system.h>
#include <time.h>
#include <solar48_config.h>
#include <crc.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <registers.h>

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1

// master mode
uint8_t modbus_master_buffer[MODBUS_ADU_MAX_SIZE];
SOLAR48_RS485_RTU master_rs485_rtu = {0};
static SOLAR48_MEM modbus_master_buffer_receive_dynamic = {0};

extern void app_panic(const char *);

static int master_prepare_to_send(
  uint8_t *out_data_size,
  uint8_t slave_address,
  MB_FUNCTION function,
  uint16_t mem_address,
  uint16_t n,
  uint16_t write_mem_address,
  uint16_t n_write,
  void *any
)
{

  if (slave_address > 247)
    return E_MASTER_INVALID_SLAVE_ADDRESS;

  modbus_master_buffer[0] = slave_address;
  PDU_FRAME *frame = (PDU_FRAME *)&modbus_master_buffer[1];

  uint16_t m, q, crc;
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

    case WRITE_SINGLE_COIL:
      if (n == 0x0000 || n == 0xFF00)
        goto master_prepare_to_send_write_discrete;

      return E_INVALID_SINGLE_COIL_STATE;

    case WRITE_SINGLE_REGISTER:
      goto master_prepare_to_send_write_discrete;

    case WRITE_MULTIPLE_COILS:
      if (n < 1 || n > 0x07B0) //1968
        return E_INVALID_WRITE_MULTIPLE_COILS_LIMIT;

      // Byte count
      m = n >> 3; // Divide by 8
      if (n & 7)
        ++m;

      // No 2 Bytes count. thus q = 0
      q = 0;

      // Limit: 1968 / 16 = 123
      // (1 (slv address) + 2 (start address) + 2 (number of registers) + 1 (byte count) + 2*123 = 252) < 256
      goto master_prepare_to_send_write;

    case WRITE_MULTIPLE_REGISTERS:
      if (n < 1 || n > 0x007B) //123
        return E_INVALID_WRITE_MULTIPLE_REGISTER_LIMIT;

      // Byte count
      m = n << 1; // Multiply by 2 => 2Bytes = (uint16_t)

      // No 2 Bytes step count. thus q != 0
      q = n;

      // Limit: 1968 / 16 = 123
      // (1 (slv address) + 2 (start address) + 2 (number of registers) + 1 (byte count) + 2*123 = 252) < 256
      goto master_prepare_to_send_write;

    case READ_OR_WRITE_MULTIPLE_REGISTERS:
      if (n < 1 || n > 0x007B)
        return E_INVALID_READ_WRITE_MULTIPLE_REGISTER_LIMIT_RD;

      if (n_write < 1 || n_write > 0x0079)
        return E_INVALID_READ_WRITE_MULTIPLE_REGISTER_LIMIT_WR;

      // Byte count
      m = n_write << 1; // Multiply by 2 => 2Bytes = (uint16_t)

      move_uint8_safe(&frame->pdu_read_write_req.function_code, function);
      swap_and_move_uint16_safe(&frame->pdu_read_write_req.read_starting_address, mem_address);
      swap_and_move_uint16_safe(&frame->pdu_read_write_req.quantity_to_read, n);
      swap_and_move_uint16_safe(&frame->pdu_read_write_req.write_starting_address, write_mem_address);
      swap_and_move_uint16_safe(&frame->pdu_read_write_req.quantity_to_write, n_write);

      move_uint8_safe(&frame->pdu_read_write_req.write_byte_count, m); // Multiply by 2 => 2Bytes = (uint16_t)

      // Copy and convert: Little Endian to Big Endian data is uint16_t *
      swap16_array_fast_safe(memcpy((void *)&frame->pdu_read_write_req.write_register_values[0], (void *)any, (size_t)m), (size_t)n_write);

      // We need to subtract 2 bytes in register_values[0] into sizeof(struct pdu_read_write_req_t). We can map it to uint8_t * or uint16_t *
      #define READ_WRITE_REGISTER_REQ_SIZE (1 + offsetof(struct pdu_read_write_req_t, write_register_values[0]) - 2)
      q = READ_WRITE_REGISTER_REQ_SIZE + m;
      crc = crc16(modbus_master_buffer, q);
      memcpy((void *)&modbus_master_buffer[(size_t)q], (void *)&crc, sizeof(crc));
      *out_data_size = (q + sizeof(crc));
      #undef READ_WRITE_REGISTER_REQ_SIZE

      return 0;

    case MASK_WRITE_REGISTER:
    case READ_FIFO_QUEUE:
    case READ_FILE_RECORD:
    case WRITE_FILE_RECORD:
    case READ_EXCEPTION_STATUS:
    case DIAGNOSTIC:
    case GET_COM_EVENT_COUNTER:
    case GET_COM_EVENT_LOG:
    case REPORT_SLAVE_ID:
    case READ_DEVICE_INDENTIFICATION:
      return E_UNIMPLEMENTED_MODBUS_FUNCTION;
    default:
      return E_UNEXPECTED_FUNCTION; // Guard
  }

master_prepare_to_send_write:

  master_rs485_rtu.first_pass = &frame->pdu_write_resp.function_code;
  master_rs485_rtu.first_pass_len = 2; // 1 (addr) + 1 (response code)
  master_rs485_rtu.second_pass = (uint8_t *)&frame->pdu_write_resp.starting_address;
  master_rs485_rtu.second_pass_len = 4; // 2 bytes + 2 bytes
  master_rs485_rtu.transfer_left_data_limit = 0; // Disable. Receive is only size of first_pass_len + second_pass_len or error packed size

  move_uint8_safe(&frame->pdu_write_req.function_code, function);
  swap_and_move_uint16_safe(&frame->pdu_write_req.starting_address, mem_address); // Big endian
  swap_and_move_uint16_safe(&frame->pdu_write_req.number_of_registers, n); // Big endian
  move_uint8_safe(&frame->pdu_write_req.byte_count, m); // m bytes

  memcpy((void *)&frame->pdu_write_req.register_values[0], (void *)any, (size_t)m);

  // Convert: Little Endian to Big Endian if q != 0. If q != 0 data is uint16_t *
  if (q)
    swap16_array_fast_safe(&frame->pdu_write_req.register_values[0], (size_t)q);

  // We need to subtract 2 register_values[0] into sizeof(struct pdu_write_req_t). We can map it to uint8_t * or uint16_t *
  #define WRITE_REGISTER_REQ_SIZE (1 + offsetof(struct pdu_write_req_t, register_values[0]) - 2)
  q = WRITE_REGISTER_REQ_SIZE + m;
  crc = crc16(modbus_master_buffer, q);
  memcpy((void *)&modbus_master_buffer[(size_t)q], (void *)&crc, sizeof(crc));
  *out_data_size = (q + sizeof(crc));
  #undef WRITE_REGISTER_REQ_SIZE

  return 0;

master_prepare_to_send_write_discrete:

  master_rs485_rtu.first_pass = &frame->pdu_write_discrete_resp.function_code;
  master_rs485_rtu.first_pass_len = 2; // 1 (addr) + 1 (response code)
  master_rs485_rtu.second_pass = (uint8_t *)&frame->pdu_write_discrete_resp.output_address_or_register_address;
  master_rs485_rtu.second_pass_len = 4; // 2 bytes + 2 bytes
  master_rs485_rtu.transfer_left_data_limit = 0; // Disable. Receive is only size of first_pass_len + second_pass_len or error packed size

  move_uint8_safe(&frame->pdu_write_discrete_req.function_code, function);
  swap_and_move_uint16_safe(&frame->pdu_write_discrete_req.output_address_or_register_address, mem_address); // Big endian
  swap_and_move_uint16_safe(&frame->pdu_write_discrete_req.output_value_or_register_value, n); // Big endian

  #define WRITE_DISCRETE_REQ_SIZE (1 + sizeof(struct pdu_write_discrete_req_t))
  crc = crc16(modbus_master_buffer, WRITE_DISCRETE_REQ_SIZE);
  memcpy((void *)&modbus_master_buffer[WRITE_DISCRETE_REQ_SIZE], (void *)&crc, sizeof(crc));
  *out_data_size = (WRITE_DISCRETE_REQ_SIZE + sizeof(crc));
  #undef WRITE_DISCRETE_REQ_SIZE

  return 0;

master_prepare_to_send_read_register:

  if (n < 1 || n > 125)
    return error_on_invalid_n;

  master_rs485_rtu.first_pass = &frame->pdu_read_resp.function_code;
  master_rs485_rtu.first_pass_len = 2; // 1 (addr) + 1 (response code)
  master_rs485_rtu.second_pass = &frame->pdu_read_resp.byte_count;
  master_rs485_rtu.second_pass_len = 1; // 1 byte count
  master_rs485_rtu.transfer_left_data_limit = 250; // 250 + 3 + 2 (crc) = 255. Guard: Don't trust, verify

  move_uint8_safe(&frame->pdu_read_req.function_code, function);
  swap_and_move_uint16_safe(&frame->pdu_read_req.starting_address, mem_address); // Big endian
  swap_and_move_uint16_safe(&frame->pdu_read_req.number_of_registers, n); // Big endian

  #define READ_REGISTER_REQ_SIZE (1 + sizeof(struct pdu_read_req_t))
  crc = crc16(modbus_master_buffer, READ_REGISTER_REQ_SIZE);
  memcpy((void *)&modbus_master_buffer[READ_REGISTER_REQ_SIZE], (void *)&crc, sizeof(crc));
  *out_data_size = (READ_REGISTER_REQ_SIZE + sizeof(crc));
  #undef READ_REGISTER_REQ_SIZE

  return 0;

master_prepare_to_send_read_discrete:

  if (n < 1 || n > 2000)
    return error_on_invalid_n; // 6.1 01 (0x01) Read Coils - Page 12, 6.2 02 (0x02) Read Discrete Inputs

  m = n >> 3; // Divide by 8
  if (n & 7) // Remainder
    ++m;

  master_rs485_rtu.first_pass = &frame->pdu_read_discrete_resp.function_code;
  master_rs485_rtu.first_pass_len = 2; // 1 (addr) + 1 (response code)
  master_rs485_rtu.second_pass = &frame->pdu_read_discrete_resp.byte_count;
  master_rs485_rtu.second_pass_len = 1; // 1 byte count
  master_rs485_rtu.transfer_left_data_limit = 250; // 250 + 3 + 2 (crc) = 255. Guard: Don't trust, verify

  move_uint8_safe(&frame->pdu_read_discrete_req.function_code, function);
  swap_and_move_uint16_safe(&frame->pdu_read_discrete_req.starting_address, mem_address); // Big endian
  swap_and_move_uint16_safe(&frame->pdu_read_discrete_req.number_of_discrete, m); // Big endian

  #define READ_DISCRETE_REQ_SIZE (1 + sizeof(struct pdu_read_discrete_req_t))
  crc = crc16(modbus_master_buffer, READ_DISCRETE_REQ_SIZE);
  memcpy((void *)&modbus_master_buffer[READ_DISCRETE_REQ_SIZE], (void *)&crc, sizeof(crc));
  *out_data_size = (READ_DISCRETE_REQ_SIZE + sizeof(crc));
  #undef READ_DISCRETE_REQ_SIZE

  return 0;
}

static struct master_rs485_exception_code_t {
  int error_on_catch_exception;
  int error_unexpected_function;
  int error_invalid_data_size_or_unexpected_discrete;
  int error_invalid_crc;
  int error_invalid_exception_crc;
} MASTER_RS485_EXCEPTION_CODE[] = {
  {
    // 0: READ_COILS: error handler
    E_UNEXPECTED_READ_COILS_ERROR_CODE,
    E_UNEXPECTED_READ_COILS_FUNCTION,
    E_INVALID_READ_COILS_DATA_SIZE,
    E_INVALID_READ_COILS_CRC,
    E_INVALID_READ_COILS_EXCEPTION_CRC
  },
  {
    // 1: READ_DISCRETE_INPUTS: error handler
    E_UNEXPECTED_READ_DISCRETE_INPUTS_ERROR_CODE,
    E_UNEXPECTED_DISCRETE_INPUTS_FUNCTION,
    E_INVALID_DISCRETE_INPUTS_DATA_SIZE,
    E_INVALID_DISCRETE_INPUTS_CRC,
    E_INVALID_DISCRETE_INPUTS_EXCEPTION_CRC
  },
  {
    // 2: READ_HOLDING_REGISTERS: error handler
    E_UNEXPECTED_READ_HOLDING_REGISTERS_ERROR_CODE,
    E_UNEXPECTED_READ_HOLDING_REGISTERS_FUNCTION,
    E_INVALID_READ_HOLDING_REGISTERS_DATA_SIZE,
    E_INVALID_READ_HOLDING_REGISTERS_CRC,
    E_INVALID_READ_HOLDING_REGISTERS_EXCEPTION_CRC
  },
  {
   // 3: READ_INPUT_REGISTERS: error handler
   E_UNEXPECTED_READ_INPUT_REGISTERS_ERROR_CODE,
   E_UNEXPECTED_READ_INPUT_REGISTERS_FUNCTION,
   E_INVALID_READ_INPUT_REGISTERS_DATA_SIZE,
   E_INVALID_READ_INPUT_REGISTERS_CRC,
   E_INVALID_READ_INPUT_REGISTERS_EXCEPTION_CRC
  },
  {
   // 4: WRITE_SINGLE_COIL: error handler
   E_UNEXPECTED_WRITE_SINGLE_COIL_ERROR_CODE,
   E_UNEXPECTED_WRITE_SINGLE_COIL_FUNCTION,
   E_INVALID_WRITE_SINGLE_COIL_DATA_SIZE,
   E_INVALID_WRITE_SINGLE_COIL_CRC,
   E_INVALID_EXCEPTION_WRITE_SINGLE_COIL_CRC_CHECK
  },
  {
   // 5: WRITE_SINGLE_REGISTER: error handler
   E_UNEXPECTED_WRITE_SINGLE_REGISTER_ERROR_CODE,
   E_UNEXPECTED_WRITE_SINGLE_REGISTER_FUNCTION,
   E_INVALID_WRITE_SINGLE_REGISTER_DATA_SIZE,
   E_INVALID_WRITE_SINGLE_REGISTER_CRC,
   E_INVALID_EXCEPTION_WRITE_SINGLE_REGISTER_CRC_CHECK
  },
  {
   // 6: WRITE_MULTIPLE_COILS: error handler
   E_UNEXPECTED_WRITE_MULTIPLE_COILS_ERROR_CODE,
   E_UNEXPECTED_WRITE_MULTIPLE_COILS_FUNCTION,
   E_INVALID_WRITE_MULTIPLE_COILS_DATA_SIZE,
   E_INVALID_WRITE_MULTIPLE_COILS_CRC,
   E_INVALID_EXCEPTION_WRITE_MULTIPLE_COILS_CRC_CHECK
  },
  {
   // 7: WRITE_MULTIPLE_REGISTERS: error handler
   E_UNEXPECTED_WRITE_MULTIPLE_REGISTERS_ERROR_CODE,
   E_UNEXPECTED_WRITE_MULTIPLE_REGISTERS_FUNCTION,
   E_INVALID_WRITE_MULTIPLE_REGISTERS_DATA_SIZE,
   E_INVALID_WRITE_MULTIPLE_REGISTERS_CRC,
   E_INVALID_EXCEPTION_WRITE_MULTIPLE_REGISTERS_CRC_CHECK
  },
  {
   // 8: READ_OR_WRITE_MULTIPLE_REGISTERS: error handler
   E_UNEXPECTED_READ_OR_WRITE_MULTIPLE_REGISTERS_ERROR_CODE,
   E_UNEXPECTED_READ_OR_WRITE_MULTIPLE_REGISTERS_FUNCTION,
   E_INVALID_READ_OR_WRITE_MULTIPLE_REGISTERS_DATA_SIZE,
   E_INVALID_READ_OR_WRITE_MULTIPLE_REGISTERS_CRC,
   E_INVALID_EXCEPTION_READ_OR_WRITE_MULTIPLE_REGISTERS_CRC_CHECK
  }
};


#define RS485_ERROR_CODE(x) (x + 0x80)
#define RS485_CHECK_EXCEPTION_CRC(type, on_error) \
memcpy(&crc, &(&frame->type)[1], sizeof(uint16_t)); \
if (crc16(modbus_master_buffer, (sizeof(frame->type) + 1)) != crc) \
  return on_error;
// Helper function. All data is read from master an checks for validation
static int master_check_receive(MB_FUNCTION *function, uint8_t **data, uint16_t *data_size)
{
  *data = NULL;
  *data_size = 0;
  *function = master_rs485_rtu.function;

  if (master_rs485_rtu.address != modbus_master_buffer[0])
    return E_MASTER_INVALID_RECEIVE_SLAVE_ADDRESS;

  bool resize_vec_size = false;
  uint16_t crc, u16_tmp;
  uint8_t 
    u8_sz,
    *ptr,
    error_code;

  struct master_rs485_exception_code_t *master_rs485_exception_code;

  int ret_code = MASTER_TRANSFER_SUCCESS;

  PDU_FRAME *frame = (PDU_FRAME *)&modbus_master_buffer[1];

  switch (master_rs485_rtu.function) {
    case READ_COILS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[0];

      goto master_check_receive_read_discrete;

    case READ_DISCRETE_INPUTS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[1];

      goto master_check_receive_read_discrete;

    case READ_HOLDING_REGISTERS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[2];

      goto master_check_receive_read_register;

    case READ_INPUT_REGISTERS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[3];

      goto master_check_receive_read_register;

    case WRITE_SINGLE_COIL:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[4];

      goto master_check_receive_write_discrete;

    case WRITE_SINGLE_REGISTER:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[5];

      goto master_check_receive_write_discrete;

    case WRITE_MULTIPLE_COILS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[6];

      goto master_check_receive_write;

    case WRITE_MULTIPLE_REGISTERS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[7];

      goto master_check_receive_write;

    case READ_OR_WRITE_MULTIPLE_REGISTERS:
      master_rs485_exception_code = &MASTER_RS485_EXCEPTION_CODE[8];

      goto master_check_receive_read_write;

    case MASK_WRITE_REGISTER:
    case READ_FIFO_QUEUE:
    case READ_FILE_RECORD:
    case WRITE_FILE_RECORD:
    case READ_EXCEPTION_STATUS:
    case DIAGNOSTIC:
    case GET_COM_EVENT_COUNTER:
    case GET_COM_EVENT_LOG:
    case REPORT_SLAVE_ID:
    case READ_DEVICE_INDENTIFICATION:
      return E_UNIMPLEMENTED_MODBUS_FUNCTION;
    default:
      return E_UNEXPECTED_RECEIVE_FUNCTION; // Guard
  }

master_check_receive_read_write:
  if (read_uint8(&frame->pdu_read_write_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    RS485_CHECK_EXCEPTION_CRC(pdu_read_write_exception, master_rs485_exception_code->error_invalid_exception_crc)

    error_code = read_uint8(&frame->pdu_read_write_exception.error_or_exception_code);
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_read_write_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_read_write_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1;
    }

    return master_rs485_exception_code->error_on_catch_exception;
  }

  if (read_uint8(&frame->pdu_read_write_resp.function_code) != master_rs485_rtu.function)
    return master_rs485_exception_code->error_unexpected_function;

  u8_sz = read_uint8(&frame->pdu_read_write_resp.quantity_read);

  if ((u8_sz < 2) || (u8_sz > 250))
    return master_rs485_exception_code->error_invalid_data_size_or_unexpected_discrete;

  // Before copy crc, we need to divide u8_sz / 2. Status type is uint16_t (2 bytes. See page 38)
  memcpy(&crc, &(((uint8_t *)&frame->pdu_read_write_resp.read_register_value)[u8_sz]), sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, (size_t)(u8_sz + offsetof(struct pdu_read_write_resp_t, read_register_value) + 1)) == crc) {
    ptr = (uint8_t *)&frame->pdu_read_write_resp.read_register_value;
    resize_vec_size = true; // master_check_receive_copy_buffer2 -> Vector is 2 bytes long
    goto master_check_receive_copy_buffer1;
  }

  return master_rs485_exception_code->error_invalid_crc;

master_check_receive_write:
  if (read_uint8(&frame->pdu_write_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    RS485_CHECK_EXCEPTION_CRC(pdu_write_error_exception, master_rs485_exception_code->error_invalid_exception_crc)

    error_code = read_uint8(&frame->pdu_write_error_exception.error_or_exception_code);
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_write_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_write_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1;
    }

    return master_rs485_exception_code->error_on_catch_exception;
  }

  if (read_uint8(&frame->pdu_write_resp.function_code) != master_rs485_rtu.function)
    return master_rs485_exception_code->error_unexpected_function;

  if (!swap_and_compare_uint16(&frame->pdu_write_resp.starting_address, master_rs485_rtu.mem_address))
    return E_RECEIVED_WRITE_ADDRESS_MISMATCH;

  u16_tmp = read_and_swap_uint16_safe(&frame->pdu_write_resp.number_of_registers_or_qty_of_outputs);

  if (master_rs485_rtu.n_data_or_discrete != u16_tmp)
    return master_rs485_exception_code->error_invalid_data_size_or_unexpected_discrete;

  #define CHECK_RECEIVE_WRITE_PACKET_SIZE (1 + sizeof(struct pdu_write_resp_t))
  memcpy(&crc, &(&frame->pdu_write_resp)[1], sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, CHECK_RECEIVE_WRITE_PACKET_SIZE) == crc) {
    ptr = (uint8_t *)&u16_tmp;
    u8_sz = (uint8_t)sizeof(((struct pdu_write_resp_t *)NULL)->number_of_registers_or_qty_of_outputs);
    goto master_check_receive_copy_buffer1;
  }
  #undef CHECK_RECEIVE_WRITE_PACKET_SIZE

  return master_rs485_exception_code->error_invalid_crc;

master_check_receive_write_discrete:
  if (read_uint8(&frame->pdu_write_discrete_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    RS485_CHECK_EXCEPTION_CRC(pdu_write_discrete_error_exception, master_rs485_exception_code->error_invalid_exception_crc)

    error_code = read_uint8(&frame->pdu_write_discrete_error_exception.error_or_exception_code);
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_write_discrete_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_write_discrete_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1;
    }

    return master_rs485_exception_code->error_on_catch_exception;
  }

  if (read_uint8(&frame->pdu_write_discrete_resp.function_code) != master_rs485_rtu.function)
    return master_rs485_exception_code->error_unexpected_function;

  if (!swap_and_compare_uint16(&frame->pdu_write_discrete_resp.output_address_or_register_address, master_rs485_rtu.mem_address))
    return E_RECEIVED_WRITE_DISCRETE_ADDRESS_MISMATCH;

  u16_tmp = read_and_swap_uint16_safe(&frame->pdu_write_discrete_resp.output_value_or_register_value);

  if (master_rs485_rtu.n_data_or_discrete != u16_tmp)
    return master_rs485_exception_code->error_invalid_data_size_or_unexpected_discrete;

  #define CHECK_RECEIVE_WRITE_DISCRETE_PACKET_SIZE (1 + sizeof(struct pdu_write_discrete_resp_t))
  memcpy(&crc, &(&frame->pdu_write_discrete_resp)[1], sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, CHECK_RECEIVE_WRITE_DISCRETE_PACKET_SIZE) == crc) {
    ptr = (uint8_t *)&u16_tmp;
    u8_sz = (uint8_t)sizeof(((struct pdu_write_discrete_resp_t *)NULL)->output_value_or_register_value);
    goto master_check_receive_copy_buffer1;
  }
  #undef CHECK_RECEIVE_WRITE_DISCRETE_PACKET_SIZE

  return master_rs485_exception_code->error_invalid_crc;

master_check_receive_read_register:
  if (read_uint8(&frame->pdu_read_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    RS485_CHECK_EXCEPTION_CRC(pdu_read_error_exception, master_rs485_exception_code->error_invalid_exception_crc)

    error_code = read_uint8(&frame->pdu_read_error_exception.error_or_exception_code);
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_read_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_read_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1; // master_check_receive_copy_buffer1 -> Size is the same as number of byte
    }

    return master_rs485_exception_code->error_on_catch_exception;
  }

  if (read_uint8(&frame->pdu_read_resp.function_code) != master_rs485_rtu.function)
    return master_rs485_exception_code->error_unexpected_function;

  u8_sz = read_uint8(&frame->pdu_read_resp.byte_count);
  // (2*125 -> See: 6.3 03 (0x03) Read Holding Registers - page 15) + sizeof(address) + sizeof(function) + sizeof(count) + sizeof(crc) = 255
  if (u8_sz < 2 || u8_sz > 250)
    return master_rs485_exception_code->error_invalid_data_size_or_unexpected_discrete;

  // Before copy crc, we need to divide u8_sz / 2. Status type is uint16_t (2 bytes. See page 15)
  memcpy(&crc, &(((uint8_t *)&frame->pdu_read_resp.status)[u8_sz]), sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, (size_t)(u8_sz + offsetof(struct pdu_read_resp_t, status) + 1)) == crc) {
    ptr = (uint8_t *)&frame->pdu_read_resp.status[0];
    resize_vec_size = true; // master_check_receive_copy_buffer2 -> Vector is 2 bytes long
    goto master_check_receive_copy_buffer1;
  }

  return master_rs485_exception_code->error_invalid_crc;

master_check_receive_read_discrete:
  if (read_uint8(&frame->pdu_read_discrete_error_exception.function_code) == RS485_ERROR_CODE(master_rs485_rtu.function)) {

    RS485_CHECK_EXCEPTION_CRC(pdu_read_discrete_error_exception, master_rs485_exception_code->error_invalid_exception_crc)

    error_code = read_uint8(&frame->pdu_read_discrete_error_exception.error_or_exception_code);
    if ((error_code > 0) && (error_code < 5)) {// Page 12
      ptr = &frame->pdu_read_discrete_error_exception.function_code;
      u8_sz = (uint8_t)sizeof(struct pdu_read_discrete_error_exception_t);
      ret_code = MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE;
      goto master_check_receive_copy_buffer1;
    }

    return master_rs485_exception_code->error_on_catch_exception;
  }

  if (read_uint8(&frame->pdu_read_discrete_resp.function_code) != master_rs485_rtu.function)
    return master_rs485_exception_code->error_unexpected_function;
        
  u8_sz = read_uint8(&frame->pdu_read_discrete_resp.byte_count);
  // (250 + 1 -> See: 6.1 01 (0x01) Read Coils - page 12) + sizeof(address) + sizeof(function) + sizeof(count) + sizeof(crc) = 256
  if (u8_sz < 1 || u8_sz > 251)
    return master_rs485_exception_code->error_invalid_data_size_or_unexpected_discrete;

  memcpy(&crc, &frame->pdu_read_discrete_resp.status[u8_sz], sizeof(uint16_t)); // Guarantees ARM alignment

  if (crc16(modbus_master_buffer, (size_t)(u8_sz + offsetof(struct pdu_read_discrete_resp_t, status) + 1)) == crc) {
    ptr = &frame->pdu_read_discrete_resp.status[0];
    goto master_check_receive_copy_buffer1;
  }

  return master_rs485_exception_code->error_invalid_crc;

master_check_receive_copy_buffer1:
  //if ((*data = (uint8_t *)malloc((size_t)u8_sz))) {
  if ((*data = (uint8_t *)solar48_mem(&modbus_master_buffer_receive_dynamic, (size_t)u8_sz))) {
    memcpy((void *)(*data), (void *)ptr, (size_t)u8_sz);

    if (resize_vec_size)
      swap16_array_fast((uint16_t *)*data, (size_t)(*data_size = (uint16_t)(u8_sz >> 1)));
    else
      *data_size = (uint16_t)u8_sz;

    return ret_code;
  }

  app_panic("mds:ckrcv1");
  return -1; // Never gets here
}

#undef RS485_ERROR_CODE

void _master_receive(int status)
{
  uint8_t *data = NULL;
  uint16_t data_size = 0;
  MB_FUNCTION func;
  switch (status) {
    case UART1_RECEIVE_COMPLETE:
      status = master_check_receive(&func, &data, &data_size);

      // STATUS = 0 => receive is valid and data is available on buffer
      if ((status == MASTER_TRANSFER_SUCCESS) || (status == MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE)) {
        // From here data is valid and formatted to Little endian in 16 bits results
        sys_unlock(&master_rs485_rtu.lock);
        master_rs485_rtu.callback(status, func, data, data_size);

        return;
      }

    default:
      if (data)
        app_panic("_mrcv:mem2");
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status, MB_FUNCTION_UNDEFINED, NULL, 0);
  }
}

static void _master_send_req(int status)
{

  if (status != UART1_TRANSFER_COMPLETE) {
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status, MB_FUNCTION_UNDEFINED, NULL, 0);
  }
}

// Implement RTU only (uses UART1)
int master_send_req(
  uint8_t slave_address,
  MB_FUNCTION function,
  uint16_t mem_address,
  uint16_t n,
  uint16_t write_start_address,
  uint16_t write_byte_count,
  void *data, // Either uint8_t * or uint16_t *
  uint32_t timeout_ms,
  mb_master_recv_callback response_callback
)
{

  TIMEOUT_MS timeout;

  if (!sys_try_lock(&master_rs485_rtu.lock, &timeout, MASTER_RS485_TIMEOUT_MS, NULL))
    return E_MASTER_MODBUS_BUSY;

  master_rs485_rtu.address = slave_address;
  master_rs485_rtu.function = function;
  master_rs485_rtu.mem_address = mem_address;
  master_rs485_rtu.write_start_address = write_start_address;
  master_rs485_rtu.write_byte_count = write_byte_count;
  master_rs485_rtu.n_data_or_discrete = n;
  master_rs485_rtu.timeout_ms = timeout_ms;
//  master_rs485_rtu.data = data;
  master_rs485_rtu.callback = response_callback;

  uint8_t out_data_size;
  int err = master_prepare_to_send(&out_data_size, slave_address, function, mem_address, n, write_start_address, write_byte_count, data);

  if (err) {
    sys_unlock(&master_rs485_rtu.lock);
    return err;
  }

  if ((err = uart1_transmit(modbus_master_buffer, (size_t)out_data_size, _master_send_req, timeout_ms)))
    sys_unlock(&master_rs485_rtu.lock);

  return err;
}

#endif

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2

#define SLAVE_READ_HOLDING_REGISTER_START_ADDRESS 0x4000
_Static_assert(((SLAVE_READ_HOLDING_REGISTER_START_ADDRESS & 1) == 0), "SLAVE_READ_HOLDING_REGISTER_START_ADDRESS must be odd");

// slave mode
uint8_t modbus_slave_buffer[MODBUS_SLAVE_BUFFER_SIZE];
SOLAR48_RS485_RTU_SLAVE slave_rs485_rtu = {0};

static void _set_slave_pdu_read_error_exception(uint8_t **data, size_t *data_size, PDU_FRAME *pdu_frame, uint8_t function_code, uint8_t exception_code)
{
#define SLAVE_PDU_ERROR_EXCEPTION_SIZE (uint16_t)(sizeof(pdu_frame->pdu_read_error_exception) + 1)

  move_uint8_safe(&pdu_frame->pdu_read_error_exception.function_code, 0x80 + function_code);
  move_uint8_safe(&pdu_frame->pdu_read_error_exception.error_or_exception_code, exception_code);

  uint16_t crc16_check = crc16(&modbus_slave_buffer[0], SLAVE_PDU_ERROR_EXCEPTION_SIZE);

  memcpy(&(&pdu_frame->pdu_read_error_exception)[1], &crc16_check, sizeof(crc16_check));

  *data = &modbus_slave_buffer[0];
  *data_size = (size_t)(SLAVE_PDU_ERROR_EXCEPTION_SIZE) + sizeof(crc16_check);

#undef SLAVE_PDU_ERROR_EXCEPTION_SIZE
}

RS485_HOLDING_REGISTERS_MEMORY_AREA rs485_slave_holding_register_memory_area = {0};
_Static_assert(4*sizeof(uint32_t) == sizeof(RS485_HOLDING_REGISTERS_MEMORY_AREA), "Refactor RS485_HOLDING_REGISTERS_MEMORY_AREA. Must be multiple of 4");
// List is 2 times greater than sectorXXX because one sector holds 2 RS485 memory address
#define RS485_HOLDING_REGISTERS_MEMORY_AREA_SIZE (sizeof(RS485_HOLDING_REGISTERS_MEMORY_AREA) / sizeof(uint16_t))

static RS485_HOLDING_REGISTERS_MEMORY_AREA rs485_slave_holding_register_memory_area_read_only;

int slave_send_resp(uint8_t **data, size_t *data_size)
{
  *data = NULL;

  if (modbus_slave_buffer[0] != slave_rs485_rtu.slave_address)
    return E_RS485_SLAVE_DOES_NOT_MATCH;

  PDU_FRAME *pdu_frame = (PDU_FRAME *)&modbus_slave_buffer[1];

  // 6.3 03 (0x03) Read Holding Registers (Page 15)
  uint16_t crc16_check;

  memcpy(&crc16_check, &(&pdu_frame->pdu_read_req)[1], sizeof(crc16_check));

  if (crc16_check == crc16(&modbus_slave_buffer[0], (uint16_t)(sizeof(pdu_frame->pdu_read_req) + 1))) {

    uint8_t function_code = read_uint8((void *)&pdu_frame->pdu_read_req.function_code);

    if (function_code != READ_HOLDING_REGISTERS) {
      _set_slave_pdu_read_error_exception(data, data_size, pdu_frame, function_code, 1);
      return E_RS485_SLAVE_FUNCTION_CODE_NOT_SUPPORTED;
    }

    uint16_t number_of_registers = read_and_swap_uint16_safe((void *)&pdu_frame->pdu_read_req.number_of_registers);

    if (number_of_registers > 125 || number_of_registers < 1) {
      _set_slave_pdu_read_error_exception(data, data_size, pdu_frame, function_code, 3);
      return E_RS485_SLAVE_QTY_OF_REGISTER_OUT_OF_BOUNDS;
    }

    uint16_t starting_address = read_and_swap_uint16_safe((void *)&pdu_frame->pdu_read_req.starting_address);

    if (((size_t)starting_address < SLAVE_READ_HOLDING_REGISTER_START_ADDRESS) ||
      (((size_t)starting_address + (size_t)number_of_registers) > (SLAVE_READ_HOLDING_REGISTER_START_ADDRESS + RS485_HOLDING_REGISTERS_MEMORY_AREA_SIZE)))
    {
      _set_slave_pdu_read_error_exception(data, data_size, pdu_frame, function_code, 2);
      return E_RS485_SLAVE_MEMORY_OUT_OF_BOUNDS;
    }

    uint16_t byte_count = (number_of_registers << 1);

    uint8_t *u16_ptr_unaligned = (uint8_t *)(&pdu_frame->pdu_read_resp.status[0]);

    starting_address -= SLAVE_READ_HOLDING_REGISTER_START_ADDRESS;

    uint8_t *mem_address_ptr = 
      (uint8_t *)&((uint16_t *)&rs485_slave_holding_register_memory_area_read_only.sector000)[(size_t)(starting_address)];

    TIMEOUT_MS timeout_ms;
    if (!sys_try_lock(&slave_rs485_rtu.lock, &timeout_ms, SLAVE_RS485_HOLDING_MEMORY_COPY_TIMEOUT_MS, NULL))
      return E_RS485_SLAVE_COPY_MEMORY_HOLDING_ADDRESS_TIMEOUT;

    memcpy(
      (void *)mem_address_ptr,
      (void *)&((uint16_t *)&rs485_slave_holding_register_memory_area.sector000)[(size_t)(starting_address)],
      (size_t)byte_count
    );

    sys_unlock(&slave_rs485_rtu.lock);

    if (starting_address & 1) {
      // check if starting_address is even
      swap_and_move_uint16_from_unaligned_to_unaligned_safe(u16_ptr_unaligned, (uint16_t *)mem_address_ptr);
      mem_address_ptr += sizeof(uint16_t); // Now mem_address_ptr is multiple of 4. Waiting to receive next byte(s) (if available)
      --number_of_registers; // One position were copied into u16_ptr_unaligned and swapped
    }

    while (number_of_registers > 1) {
      swap_and_move_two_uint16_at_once_safe((void *)u16_ptr_unaligned, *((uint32_t *)mem_address_ptr));

      mem_address_ptr += 4; // Advances 4 bytes (2 x uint16_t)
      number_of_registers -= 2;
    }

    // Last uint16_t left (if exists)
    if (number_of_registers == 1)
      swap_and_move_uint16_safe(u16_ptr_unaligned, *((uint16_t *)mem_address_ptr));

    move_uint8_safe(&pdu_frame->pdu_read_resp.byte_count, (uint8_t)byte_count);

    #define SLAVE_PDU_READ_RESP_SIZE (size_t)(1 + &((struct pdu_read_resp_t *)NULL)->status[0])
    byte_count += (uint16_t)SLAVE_PDU_READ_RESP_SIZE;
    #undef SLAVE_PDU_READ_RESP_SIZE

    *data = &modbus_slave_buffer[0];

    crc16_check = crc16(*data, byte_count);

    memcpy(&modbus_slave_buffer[byte_count], &crc16_check, sizeof(crc16_check));

    *data_size = byte_count + sizeof(crc16_check);

    return 0;
  }

  return E_RS485_SLAVE_INVALID_CRC16;

}

void rs485_slave_transmit_callback(int error)
{
  error_handler(error);
}

void rs485_slave_receive_callback(int error)
{
  error_handler(error);
}

#endif

