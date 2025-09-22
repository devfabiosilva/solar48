// References: https://epics-modbus.readthedocs.io/en/latest/overview.html

#include <rs485.h>
#include <hal_uart.h>


static uint8_t modbus_master_buffer[256]; 

// Implement RTU only (uses UART1)
int master_send_req(uint16_t slave_address, MB_FUNCION function, uint16_t mem_address, uint8_t n, mb_master_callback response_callback)
{
  //TODO implement
  return 0;
}

