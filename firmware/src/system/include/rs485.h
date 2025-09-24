#ifndef RS485_H
 #define RS485_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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
typedef enum solar48_rs485_e {
  READ_DISCRETE_INPUTS = 2,
  READ_COILS = 1,
  WRITE_SINGLE_COIL = 5,
  WRITE_MULTIPLE_COILS = 15,
  READ_INPUT_REGISTER = 4,
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
  READ_DEVICE_INDENTIFICATION = 43
} MB_FUNCION;

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
  uint16_t number_of_registers;
}__attribute__((packed));

struct pdu_write_error_exception_t {
  uint8_t error_or_exception_code;
}__attribute__((packed));
// End

typedef void (*mb_callback)(int);

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
} PDU_FRAME;

typedef struct solar48_rs485rtu_t {
  volatile bool lock;
  uint8_t address;
  MB_FUNCION function;
  uint32_t timeout_ms;
  mb_callback callback;
} SOLAR48_RS485_RTU;

int master_send_req(uint8_t, MB_FUNCION, uint16_t, uint16_t, uint32_t, mb_callback);

#endif

