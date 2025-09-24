// References: https://epics-modbus.readthedocs.io/en/latest/overview.html

#include <rs485.h>
#include <hal_uart.h>
#include <system.h>
#include <time.h>
#include <solar48_config.h>
#include <crc.h>

static uint8_t modbus_master_buffer[256];
static uint8_t modbus_slave_buffer[256];

static SOLAR48_RS485_RTU master_rs485_rtu = {0};
static SOLAR48_RS485_RTU slave_rs485_rtu = {0};

static inline uint16_t swap16(uint16_t x) {
    uint16_t y;
    __asm__("rev16 %0, %1" : "=r"(y) : "r"(x));
    return y;
}

static void swap16_array_fast(uint16_t *arr, size_t len)
{

// rev16 instructions swaps 2 half words, so calling this is faster than simple swap bytes using C.
// Examples a = 0x01020304
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
  if (len & 1) {
    uint32_t left = (uint32_t)arr[len - 1];
    __asm__("rev16 %0, %0": "=r"(left): "0"(left)); // Last swap
    arr[len - 1] = (uint16_t)left;
  }
}

static int master_prepare_to_send(uint8_t *out_data_size, uint8_t slave_address, MB_FUNCION function, uint16_t mem_address, uint16_t n)
{

  if (slave_address > 247)
    return E_MASTER_INVALID_SLAVE_ADDRESS;

  modbus_master_buffer[0] = slave_address;
  PDU_FRAME *frame = (PDU_FRAME *)&modbus_master_buffer[1];

  switch (function) {
    case READ_DISCRETE_INPUTS:
      // TODO implement
      break;
    case READ_COILS:
      if (n < 1 || n > 2000) // 6.1 01 (0x01) Read Coils - Page 12
        return E_MASTER_INVALID_NUMBER_OF_COILS;

      uint16_t m = n >> 3; // Divide by 8
      if (n & 7) // Remainder
        ++m;

      frame->pdu_read_discrete_req.function_code = function;
      frame->pdu_read_discrete_req.starting_address = swap16(mem_address); // Big endian
      frame->pdu_read_discrete_req.number_of_discrete = swap16(m); // Big endian

      #define READ_COILS_REQ_SIZE (1 + sizeof(struct pdu_read_discrete_req_t))
      *((uint16_t *)&modbus_master_buffer[READ_COILS_REQ_SIZE]) = crc16(modbus_master_buffer, READ_COILS_REQ_SIZE);
      *out_data_size = (READ_COILS_REQ_SIZE + 2);
      #undef READ_COILS_REQ_SIZE
      break;
//    default:
  }

  return 0;
}

static int master_check_receive()
{
  //TODO validate Receiving according to specification
  return 0;
}

static void _master_receive(int status)
{
  switch (status) {
    case UART1_RECEIVE_COMPLETE:
      status = master_check_receive();
    default:
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status);
  }
}

static void _master_send_req(int status)
{
  switch (status) {
    case UART1_TRANSFER_COMPLETE:
      // TODO implement slave receive buffer callback
      // TODO implement MAX485 implementation
      int err = uart1_receive(modbus_master_buffer, sizeof(modbus_master_buffer), _master_receive, master_rs485_rtu.timeout);
      if (!(err))
        return;

    default:
      sys_unlock(&master_rs485_rtu.lock);
      master_rs485_rtu.callback(status);
  }
}

// Implement RTU only (uses UART1)
int master_send_req(uint8_t slave_address, MB_FUNCION function, uint16_t mem_address, uint16_t n, uint32_t timeout_ms, mb_callback response_callback)
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

  if (!(err = uart1_transmit(modbus_master_buffer, out_data_size, _master_send_req, timeout_ms)))
    return 0;

  sys_unlock(&master_rs485_rtu.lock);
  return err;
}

