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
  size_t i = 0;
  for (; i + 3 < len; i += 4) {
    uint32_t *p32 = (uint32_t *)&arr[i];
    uint32_t *q32 = (uint32_t *)&arr[i + 2];

    __asm__(
        "rev %0, %0\n\t"
        "rev16 %0, %0"
      : "=r"(*p32)
      : "0"(*p32)
    );

    __asm__(
        "rev %0, %0\n\t"
        "rev16 %0, %0"
      : "=r"(*q32)
      : "0"(*q32)
    );
  }

  for (; i < len; i++)
    __asm__("rev16 %0, %0" : "=r"(arr[i]) : "0"(arr[i]));
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

static void _master_send_req(int status)
{
  switch (status) {
    case UART1_TRANSFER_COMPLETE:
      // TODO implement slave receive buffer callback
      sys_unlock(&master_rs485_rtu.lock); // TODO remove unlock here
      break;
    default:
      //TODO implement error handler
      sys_unlock(&master_rs485_rtu.lock);
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

