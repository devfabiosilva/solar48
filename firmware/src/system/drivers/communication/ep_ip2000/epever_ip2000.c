#include <solar48_config.h>
#include <rs485.h>
#include <drivers/communication/epever_ip2000.h>
#include <stdbool.h>
#include <system.h>
#include <time.h>
#include <string.h>
#include <types.h>
#include <stdio.h>
#include <errors.h>

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

      //TODO: Check correct size, Must be reimplemented
      if (EP_IP2000_ELEM_SIZE == data_size) // Is redundant. ModBus checker guarantees that element has same size
        memcpy((void *)&ip2000_record, (void *)data, sizeof(ip2000_record)); // TODO REFACTOR. Avoid unalign implementation
      else
        ep_ip2000err = E_EP_IP2000_READ_SENSORS_ELEM_NOT_MATCH;

    default:
      if (ep_ip2000callback)
        ep_ip2000callback(&ep_ip2000err, &ip2000_record);
      else
        error_handler(E_EP_ILLEGAL_IP2000_EP_IP2000_RECEIVE);

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
      memcpy((void *)&ep_ip2000status, (void *)data, sizeof(ep_ip2000status)); // TODO REFACTOR. Avoid unalign implementation  
    else
      ep_ip2000err = E_EP_IP2000_READ_STATUS_ELEM_NOT_MATCH;

    default:
      if (ep_ip2000status_callback)
        ep_ip2000status_callback(&ep_ip2000err, &ep_ip2000status);
      else
        error_handler(E_EP_ILLEGAL_IP2000_EP_IP2000_STATUS_RECEIVE);

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
      if (ep_ip2000device_over_temp_callback)
        ep_ip2000device_over_temp_callback(&ep_ip2000err, &ep_ip2000over_temperature);
      else
        error_handler(E_EP_ILLEGAL_IP2000_EP_IP2000_OVR_TEMP_RECEIVE);

      ep_ip2000device_over_temp_callback = NULL;
      sys_unlock(&ep2000_lock);
  }
}

int read_ep2000(ep_ip2000cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

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

char *ep2000_as_json(char *buf, size_t buf_sz, int *len)
{
  #define _buf_len 8
  char input_voltage[_buf_len];
  char input_current[_buf_len];
  char input_power[_buf_len];
  char output_voltage[_buf_len];
  char output_current[_buf_len];
  char output_power[_buf_len];
  char heat_sink_temp[_buf_len];

  #define set_ep2000_as_json(val) real_u32_prec(val, sizeof(val), NULL, ip2000_record.val, 100)

  int len_or_error = snprintf(buf, buf_sz,
    "{\"InputVoltage\": %s,"\
    "\"InputCurrent\": %s,"\
    "\"InputPower\": %s,"\
    "\"OutputVoltage\": %s,"\
    "\"OutputCurrent\": %s,"\
    "\"OutputPower\": %s,"\
    "\"HeatSinkTemp\": %s"
    "}",
    set_ep2000_as_json(input_voltage),
    set_ep2000_as_json(input_current),
    set_ep2000_as_json(input_power),
    set_ep2000_as_json(output_voltage),
    set_ep2000_as_json(output_current),
    set_ep2000_as_json(output_power),
    set_ep2000_as_json(heat_sink_temp)
  );

  #undef set_ep2000_as_json
  #undef _buf_len

  if (len_or_error < 0) {
    len_or_error = E_EP_IP2000_JSON_UNABLE_TO_PARSE;
    goto ep2000_as_json_error;
  }

  if ((size_t)len_or_error >= buf_sz) {
    len_or_error = E_EP_IP2000_JSON_BUF_OVERFLOW;

ep2000_as_json_error:
    error_handler(len_or_error);
    len_or_error = 0;
  }

  buf[len_or_error] = 0;

  if (len)
    *len = len_or_error;

  return buf;
}

int read_ep2000_status(ep_ip2000status_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, wait_unlock_timeout, NULL)) {
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

#define T "true"
#define F "false"

char *ep2000_status_as_json(char *buf, size_t buf_sz, int *len)
{

  char *input_voltage_status;
  switch (ep_ip2000status >> 14) {
    case 0:
      input_voltage_status = "Normal input voltage";
      break;
    case 1:
      input_voltage_status = "Low input voltage";
      break;
    case 2:
      input_voltage_status = "High input voltage";
      break;
    default:
      input_voltage_status = "No connect to the input power";
  }

  char *output_power_status;
  switch ((ep_ip2000status >> 12) & 3) {
    case 0:
      output_power_status = "Light load";
      break;
    case 1:
      output_power_status = "Medium load";
      break;
    case 2:
      output_power_status = "Nominal laod";
      break;
    default:
      output_power_status = "Overload";
  }

  char *state = (ep_ip2000status & (1<<0))?"Run":"StandBy";

  char *status = (ep_ip2000status & (1<<1))?"Faults":"Normal";

  char *output_fail = (ep_ip2000status & (1<<5))?T:F;

  char *high_voltage_side_short_circuit = (ep_ip2000status & (1<<6))?T:F;

  char *input_over_current = (ep_ip2000status & (1<<7))?T:F;

  char *abnormal_output_voltage = (ep_ip2000status & (1<<8))?T:F;

  char *unable_to_stop_discharging = (ep_ip2000status & (1<<9))?T:F;

  char *unable_to_discharge = (ep_ip2000status & (1<<10))?T:F;

  char *short_circuit = (ep_ip2000status & (1<<11))?T:F;

  int len_or_error = snprintf(buf, buf_sz,
    "{\"InputVoltageStatus\": %s,"\
    "\"OutputPowerStatus\": %s,"
    "\"State\": %s,"
    "\"Status\": %s,"
    "\"OutputFail\": %s,"
    "\"HighVoltageSideShortCircuit\": %s,"
    "\"InputOverCurrent\": %s,"
    "\"AbnormalOutputVoltage\": %s,"
    "\"UnableToStopDischarging\": %s,"
    "\"UnableToDischarge\": %s,"
    "\"ShortCircuit\": %s"
    "}",
     input_voltage_status,
     output_power_status,
     state,
     status,
     output_fail,
     high_voltage_side_short_circuit,
     input_over_current,
     abnormal_output_voltage,
     unable_to_stop_discharging,
     unable_to_discharge,
     short_circuit
  );

  if (len_or_error < 0) {
    len_or_error = E_EP_IP2000_STATUS_JSON_UNABLE_TO_PARSE;
    goto ep2000_status_as_json_error;
  }

  if ((size_t)len_or_error >= buf_sz) {
    len_or_error = E_EP_IP2000_STATUS_JSON_BUF_OVERFLOW;

ep2000_status_as_json_error:
    error_handler(len_or_error);
    len_or_error = 0;
  }

  buf[len_or_error] = 0;

  if (len)
    *len = len_or_error;

  return buf;
}

int read_ep2000_over_temperature(ep_ip2000device_over_temp_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, wait_unlock_timeout, NULL)) {
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

char *read_ep2000_over_temperature_as_json(char *buf, size_t buf_sz, int *len)
{
  int len_or_error = snprintf(buf, buf_sz, 
    "{\"OverTemperature\": %s}",
    (ep_ip2000over_temperature & 1)?T:F
  );

  if (len_or_error < 0) {
    len_or_error = E_EP_IP2000_OVER_TEMP_JSON_UNABLE_TO_PARSE;
    goto read_ep2000_over_temperature_as_json_error;
  }

  if ((size_t)len_or_error >= buf_sz) {
    len_or_error = E_EP_IP2000_OVER_TEMP_JSON_BUF_OVERFLOW;

read_ep2000_over_temperature_as_json_error:
    error_handler(len_or_error);
    len_or_error = 0;
  }

  buf[len_or_error] = 0;

  if (len)
    *len = len_or_error;

  return buf;
}

#undef F
#undef T

static ep_ip2000coils_read_write_cb ep_ip2000coils_read_write_callback = NULL;
static uint16_t ep_ip2000_coil_rd_wr_value = 0;

static void rs485_ep_ip2000_receive_read_write_coils_callback(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  switch (ep_ip2000err = status) {
    case MASTER_TRANSFER_SUCCESS:

      if (data_size == 1)// Is redundant. ModBus checker guarantees that element has same size
        memcpy((void *)&ep_ip2000_coil_rd_wr_value, (void *)data, sizeof(uint16_t));
      else
        ep_ip2000err = E_EP_IP2000_READ_WRITE_COIL_NOT_MATCH;

    default:
      if (ep_ip2000coils_read_write_callback)
        ep_ip2000coils_read_write_callback(&ep_ip2000err, &ep_ip2000_coil_rd_wr_value);
      else
        error_handler(E_EP_ILLEGAL_IP2000_EP_IP2000_RD_WR_COIL_RECEIVE);

      ep_ip2000coils_read_write_callback = NULL;
      sys_unlock(&ep2000_lock);
  }
}

int read_ep2000_read_coil(uint8_t mem_address, ep_ip2000coils_read_write_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, wait_unlock_timeout, NULL)) {
    ep_ip2000coils_read_write_callback = callback;
    ep_ip2000err = MASTER_READ_COILS(EPEVER_IP2000_SLAVE_ADDRESS, mem_address, 1, EPEVER_IP2000_TIMEOUT, rs485_ep_ip2000_receive_read_write_coils_callback); 

    if (ep_ip2000err) {
      ep_ip2000coils_read_write_callback = NULL;
      sys_unlock(&ep2000_lock);
    }

    return ep_ip2000err;
  }

  return E_EP_IP2000_READ_COIL_BUSY;
}

int read_ep2000_write_coil(uint8_t mem_address, uint16_t value, ep_ip2000coils_read_write_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  if (sys_try_lock(&ep2000_lock, &timeout_ms, wait_unlock_timeout, NULL)) {
    ep_ip2000coils_read_write_callback = callback;
    ep_ip2000err = MASTER_WRITE_SINGLE_COIL(EPEVER_IP2000_SLAVE_ADDRESS, mem_address, value, EPEVER_IP2000_TIMEOUT, rs485_ep_ip2000_receive_read_write_coils_callback); 

    if (ep_ip2000err) {
      ep_ip2000coils_read_write_callback = NULL;
      sys_unlock(&ep2000_lock);
    }

    return ep_ip2000err;
  }

  return E_EP_IP2000_WRITE_COIL_BUSY;
}

