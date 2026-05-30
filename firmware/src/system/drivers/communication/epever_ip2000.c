#include <rs485.h>
#include <epever_ip2000.h>
#include <stdbool.h>
#include <system.h>
#include <time.h>

static EP_IP2000 ip2000_record = {0};
static volatile bool ep2000_lock = false;
static int ep_ip2000err = 0;
static ep_ip2000cb ep_ip2000callback = NULL;
static ep_ip2000status_cb ep_ip2000status_callback = NULL;
static uint16_t ep_ip2000status = 0;
static uint16_t ep_ip2000over_temperature = 0;
static ep_ip2000device_over_temp_cb ep_ip2000device_over_temp_callback = NULL;

static void rs485_ep_ip2000_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  switch (ep_ip2000err = status) {
    case MASTER_TRANSFER_SUCCESS:

      if (EP_IP2000_ELEM_SIZE == data_size) // Is redundant. ModBus checker guarantees that element has same size
        memcpy((void *)&ip2000_record, (void *)data, sizeof(ip2000_record));
      else
        ep_ip2000err = E_EP_IP2000_READ_SENSORS_ELEM_NOT_MATCH;

    default:
      ep_ip2000callback(&ep_ip2000err, &ip2000_record);
      ep_ip2000callback = NULL;
      sys_unlock(&ep2000_lock);
  }
}

static void rs485_ep_ip2000_receive_status(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  switch (ep_ip2000err = status) {
    case MASTER_TRANSFER_SUCCESS:

      if (data_size == 1)// Is redundant. ModBus checker guarantees that element has same size
        memcpy((void *)&ep_ip2000status, (void *)data, sizeof(ep_ip2000status));
        
     else
        ep_ip2000err = E_EP_IP2000_READ_STATUS_ELEM_NOT_MATCH;

    default:
      ep_ip2000status_callback(&ep_ip2000err, &ep_ip2000status);
      ep_ip2000status_callback = NULL;
      sys_unlock(&ep2000_lock);
  }
}

static void rs485_ep_ip2000_receive_over_temperature(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  switch (ep_ip2000err = status) {
    case MASTER_TRANSFER_SUCCESS:

      if (data_size == 1)// Is redundant. ModBus checker guarantees that element has same size
        memcpy((void *)&ep_ip2000over_temperature, (void *)data, sizeof(ep_ip2000over_temperature));
      else
        ep_ip2000err = E_EP_IP2000_READ_OVERTEMP_ELEM_NOT_MATCH;

    default:
      rs485_ep_ip2000_receive_over_temperature(&ep_ip2000err, &ep_ip2000over_temperature);
      rs485_ep_ip2000_receive_over_temperature = NULL;
      sys_unlock(&ep2000_lock);
  }
}

int read_ep2000(ep_ip2000cb callback, uint32_t timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, EPEVER_IP2000_TIMEOUT, NULL)) {

    ep_ip2000callback = callback;
    ep_ip2000err = MASTER_READ_INPUT_REGISTERS(EPEVER_IP2000_SLAVE_ADDRESS, LOAD_INPUT_VOLTAGE, EP_IP2000_ELEM_SIZE, EPEVER_IP2000_TIMEOUT, rs485_ep_ip2000_receive); 

    if (ep_ip2000err) {
      ep_ip2000callback = NULL;
      sys_unlock(&ep2000_lock);
    }

    return ep_ip2000err;
  }

  return E_EP_IP2000_READ_SENSORS_BUSY;
}

int read_ep2000_status(ep_ip2000status_cb callback, uint32_t timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, EPEVER_IP2000_TIMEOUT, NULL)) {
    ep_ip2000status_callback = callback;
    ep_ip2000err = MASTER_READ_INPUT_REGISTERS(EPEVER_IP2000_SLAVE_ADDRESS, LOAD_STATUS, 1, EPEVER_IP2000_TIMEOUT, rs485_ep_ip2000_receive_status); 

    if (ep_ip2000err) {
      ep_ip2000status_callback = NULL;
      sys_unlock(&ep2000_lock);
    }

    return ep_ip2000err;
  }

  return E_EP_IP2000_READ_STATUS_BUSY;
}

int read_ep2000_over_temperature(ep_ip2000device_over_temp_cb callback, uint32_t timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, EPEVER_IP2000_TIMEOUT, NULL)) {
    ep_ip2000device_over_temp_callback = callback;
    ep_ip2000err = MASTER_READ_DISCRETE_INPUTS(EPEVER_IP2000_SLAVE_ADDRESS, LOAD_STATUS, 1, EPEVER_IP2000_TIMEOUT, rs485_ep_ip2000_receive_over_temperature); 

    if (ep_ip2000err) {
      ep_ip2000device_over_temp_callback = NULL;
      sys_unlock(&ep2000_lock);
    }

    return ep_ip2000err;
  }

  return E_EP_IP2000_READ_OVER_TEMPERATURE_BUSY;
}

