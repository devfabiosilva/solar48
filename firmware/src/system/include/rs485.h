#ifndef RS485_H
 #define RS485_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <hal_uart.h>

// Page 5
//The size of the MODBUS PDU is limited by the size constraint inherited from the first
//MODBUS implementation on Serial Line network (max. RS485 ADU = 256 bytes).
//Therefore:
//MODBUS PDU for serial line communication = 256 - Server address (1 byte) - CRC (2
//bytes) = 253 bytes.
#define MODBUS_ADU_MAX_SIZE 256U
#define MODBUS_PDU_MAX_SIZE (MODBUS_ADU_MAX_SIZE - 3)


// Modbus_Application_Protocol_V1_1b.pdf 
// 5.1 Public Function Code Definition Page 11
typedef enum solar48_functions_rs485_e {
  READ_DISCRETE_INPUTS = 2,
  READ_COILS = 1,
  WRITE_SINGLE_COIL = 5,
  WRITE_MULTIPLE_COILS = 15,
  READ_INPUT_REGISTERS = 4,
  READ_HOLDING_REGISTERS = 3,
  WRITE_SINGLE_REGISTER = 6,
  WRITE_MULTIPLE_REGISTERS = 16,
  READ_OR_WRITE_MULTIPLE_REGISTERS = 23,
  MASK_WRITE_REGISTER = 22,
  READ_FIFO_QUEUE = 24,
  READ_FILE_RECORD = 20,
  WRITE_FILE_RECORD = 21,
  READ_EXCEPTION_STATUS = 7,
  DIAGNOSTIC = 8,
  GET_COM_EVENT_COUNTER = 11,
  GET_COM_EVENT_LOG = 12,
  REPORT_SLAVE_ID = 17,
  READ_DEVICE_INDENTIFICATION = 43,
  MB_FUNCTION_UNDEFINED = 0
} MB_FUNCTION;

// Read discrete
struct pdu_read_discrete_req_t {
  uint8_t function_code;
  uint16_t starting_address;
  uint16_t number_of_discrete;
}__attribute__((packed));

struct pdu_read_discrete_resp_t {
  uint8_t function_code;
  uint8_t byte_count;
  uint8_t status[MODBUS_PDU_MAX_SIZE - 2];
}__attribute__((packed));
_Static_assert(sizeof(struct pdu_read_discrete_resp_t) == MODBUS_PDU_MAX_SIZE);

struct pdu_read_discrete_error_exception_t {
  uint8_t function_code;
  uint8_t error_or_exception_code;
}__attribute__((packed));
// End read discrete

// Read
struct pdu_read_req_t {
  uint8_t function_code;
  uint16_t starting_address;
  uint16_t number_of_registers;
}__attribute__((packed));

struct pdu_read_resp_t {
  uint8_t function_code;
  uint8_t byte_count;
  uint16_t status[(MODBUS_PDU_MAX_SIZE >> 1) - 1];
}__attribute__((packed));
_Static_assert(sizeof(struct pdu_read_resp_t) == MODBUS_PDU_MAX_SIZE - 1, "pdu_read_resp_t size error");

struct pdu_read_error_exception_t {
  uint8_t function_code;
  uint8_t error_or_exception_code;
}__attribute__((packed));
// End

//Write discrete
struct pdu_write_discrete_req_t {
  uint8_t function_code;
  uint16_t output_address_or_register_address;
  uint16_t output_value_or_register_value;
}__attribute__((packed));

struct pdu_write_discrete_resp_t {
  uint8_t function_code;
  uint16_t output_address_or_register_address;
  uint16_t output_value_or_register_value;
}__attribute__((packed));

struct pdu_write_discrete_error_exception_t {
  uint8_t function_code;
  uint8_t error_or_exception_code;
}__attribute__((packed));
//End

// Write
struct pdu_write_req_t {
  uint8_t function_code;
  uint16_t starting_address;
  uint16_t number_of_registers;
  uint8_t byte_count;
  uint16_t register_values[123];
}__attribute__((packed));
_Static_assert(sizeof(struct pdu_write_req_t) == MODBUS_PDU_MAX_SIZE - 1, "pdu_write_req_t size error");

struct pdu_write_resp_t {
  uint8_t function_code;
  uint16_t starting_address;
  uint16_t number_of_registers_or_qty_of_outputs;
}__attribute__((packed));

struct pdu_write_error_exception_t {
  uint8_t function_code;
  uint8_t error_or_exception_code;
}__attribute__((packed));
// End

//Read/Write
struct pdu_read_write_req_t {
  uint8_t function_code;
  uint16_t read_starting_address;
  uint16_t quantity_to_read;
  uint16_t write_starting_address;
  uint16_t quantity_to_write;
  uint8_t write_byte_count;
  uint16_t write_register_values[121];
}__attribute__((packed));
_Static_assert(sizeof(struct pdu_write_req_t) == MODBUS_PDU_MAX_SIZE - 1, "pdu_read_write_req_t size error");

struct pdu_read_write_resp_t {
  uint8_t function_code;
  uint8_t quantity_read;
  uint16_t read_register_value;
}__attribute__((packed));

struct pdu_read_write_exception_t {
  uint8_t function_code;
  uint8_t error_or_exception_code;
}__attribute__((packed));
//End

typedef void (*mb_master_recv_callback)(int, MB_FUNCTION, uint8_t *, uint16_t);

typedef union pdu_u {
  struct pdu_read_discrete_req_t pdu_read_discrete_req;
  struct pdu_read_discrete_resp_t pdu_read_discrete_resp;
  struct pdu_read_discrete_error_exception_t pdu_read_discrete_error_exception;

  struct pdu_read_req_t pdu_read_req;
  struct pdu_read_resp_t pdu_read_resp;
  struct pdu_read_error_exception_t pdu_read_error_exception;

  struct pdu_write_discrete_req_t pdu_write_discrete_req;
  struct pdu_write_discrete_resp_t pdu_write_discrete_resp;
  struct pdu_write_discrete_error_exception_t pdu_write_discrete_error_exception;

  struct pdu_write_req_t pdu_write_req;
  struct pdu_write_resp_t pdu_write_resp;
  struct pdu_write_error_exception_t pdu_write_error_exception;

  struct pdu_read_write_req_t pdu_read_write_req;
  struct pdu_read_write_resp_t pdu_read_write_resp;
  struct pdu_read_write_exception_t pdu_read_write_exception;
} PDU_FRAME;

typedef struct solar48_rs485rtu_t {
  volatile bool lock;
  uint8_t address;
  MB_FUNCTION function;
  uint16_t mem_address; // AKA Starting Address
  uint16_t write_start_address; // AKA Write Starting Address for Read/Write Multiple registers function
  uint16_t n_data_or_discrete;
  uint16_t write_byte_count; // AKA Write Byte Count for Read/Write Multiple registers function
  uint32_t timeout_ms;
  mb_master_recv_callback callback;
  // Receive processing
  uint8_t *first_pass; // First pass: check error or continue
  uint8_t first_pass_len; // First pass length
  uint8_t *second_pass; // Second pass: check received data length and calculate dinamically
  uint8_t second_pass_len; // First pass
  // transfer_left_data_limit (master mode): is the left N data left - 2 bytes limit to inform
  // DMA to reload the rest of incoming data. 0 -> if not zero, expected_receive_data 
  //is the size of data length pointer in request
  uint8_t transfer_left_data_limit;
} SOLAR48_RS485_RTU;

int master_send_req(
  uint8_t,
  MB_FUNCTION,
  uint16_t,
  uint16_t,
  uint16_t,
  uint16_t,
  void *, // Either uint8_t * or uint16_t *
  uint32_t,
  mb_master_recv_callback
);

#define RS485_OK UART_OK
#define MASTER_TRANSFER_SUCCESS 0
#define MASTER_TRANSFER_SUCCESS_WITH_ERROR_CODE 1

#define MASTER_RS485_DRIVER_TRANSMIT_MODE GPIOA_ODR |= (1<<ODR8);
#define MASTER_RS485_DRIVER_RECEIVE_MODE GPIOA_ODR &= ~(1<<ODR8);

#endif

