#include <stdbool.h>
#include <rs485.h>
#include <system.h>
#include <time.h>
#include <drivers/communication/epever_tracer6415an.h>
#include <string.h>

static EP_TRACER6415AN_RATED_DATUM epever_tracer6415an_rated_datum_record = {0};
static volatile bool epever_tracer6415an_lock = false;
static int epever_tracer6415an_err = 0;
static epever_tracer6415an_read_rated_datum_cb tracer6415an_read_rated_datum_cb = NULL;

#define TRACER6415AN_COPY_AND_FINISH(parent, dest) \
  memcpy((void *)&parent.dest, (void *)data, sizeof(parent.dest));

#define TRACER6415AN_COPY_AND_ADVANCE_N(parent, dest, n) \
  TRACER6415AN_COPY_AND_FINISH(parent, dest) \
  data += n;

#define TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(dest) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_rated_datum_record, dest, sizeof(epever_tracer6415an_rated_datum_record.dest))

#define TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_U16(dest, n) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_rated_datum_record, dest, (sizeof(uint16_t)*(n)))

#define TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_FINISH(dest) \
  TRACER6415AN_COPY_AND_FINISH(epever_tracer6415an_rated_datum_record, dest)

static void rs485_epever_tracer6415an_read_rated_datum_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  if ((epever_tracer6415an_err = status) == MASTER_TRANSFER_SUCCESS) {
      if (EP_TRACER6415AN_RATED_DATUM_ELEMENTS == data_size) { // Is redundant. ModBus checker guarantees that element has same size

        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(array_rated_votage)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(array_rated_current)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(array_rated_power)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(battery_rated_voltage)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(battery_rated_current)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE(battery_rated_power)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_U16(charging_mode, 0x300E - 0x3008)
        TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_FINISH(rated_current_load)

        epever_tracer6415an_err = 0; // Success
      } else
        epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_ELEM_NOT_MATCH;
  }

  tracer6415an_read_rated_datum_cb(&epever_tracer6415an_err, &epever_tracer6415an_rated_datum_record);
  tracer6415an_read_rated_datum_cb = NULL;
  sys_unlock(&epever_tracer6415an_lock);
}

#undef TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE
#undef TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_U16
#undef TRACER6415AN_RATED_DATUM_COPY_AND_ADVANCE_FINISH

int rs485_epever_tracer6415an_read_rated_datum(epever_tracer6415an_read_rated_datum_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  tracer6415an_read_rated_datum_cb = NULL;
  if (sys_try_lock(&epever_tracer6415an_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

    tracer6415an_read_rated_datum_cb = callback;
    epever_tracer6415an_err = MASTER_READ_INPUT_REGISTERS(EPEVER_TRACER6414AN_SLAVE_ADDRESS, RATED_DATUM_INITIAL_ADDRESS, EP_TRACER6415AN_RATED_DATUM_ELEMENTS, EPEVER_TRACER6414AN_TIMEOUT, rs485_epever_tracer6415an_read_rated_datum_receive); 

    if (epever_tracer6415an_err) {
      tracer6415an_read_rated_datum_cb = NULL;
      sys_unlock(&epever_tracer6415an_lock);
    }

  } else
    epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_READ_RATED_DATUM_BUSY;

  return epever_tracer6415an_err;
}

#define TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(dest) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_real_time_data_record, dest, sizeof(epever_tracer6415an_real_time_data_record.dest))

#define TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_U16(dest, n) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_real_time_data_record, dest, (sizeof(uint16_t)*(n)))

#define TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_FINISH(dest) \
  TRACER6415AN_COPY_AND_FINISH(epever_tracer6415an_real_time_data_record, dest)

static EP_TRACER6415AN_REAL_TIME_DATA epever_tracer6415an_real_time_data_record =  {0};
static epever_tracer6415an_real_time_data_cb tracer6415an_real_time_data_cb = NULL;

static void rs485_epever_tracer6415an_real_time_data_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  if ((epever_tracer6415an_err = status) == MASTER_TRANSFER_SUCCESS) {
      if (EP_TRACER6415AN_REAL_TIME_DATA_ELEMENTS == data_size) { // Is redundant. ModBus checker guarantees that element has same size

        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(pv_array_input_voltage)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(pv_array_input_current)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_U16(pv_array_input_power, 0x3106 - 0x3103)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_U16(battery_power, 0x310C - 0x3107)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(load_voltage)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(load_current)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(load_power)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(battery_temperature)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_U16(temperature_inside_equipament, 0x311A - 0x3111)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE(battery_soc)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_U16(remote_battery_temperature, 0x311D - 0x311B)
        TRACER6415AN_REAL_TIME_DATA_COPY_AND_ADVANCE_FINISH(battery_real_rated_power)

        epever_tracer6415an_err = 0; // Success
      } else
        epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_REAL_TIME_ELEM_NOT_MATCH;
  }

  tracer6415an_real_time_data_cb(&epever_tracer6415an_err, &epever_tracer6415an_real_time_data_record);
  tracer6415an_real_time_data_cb = NULL;
  sys_unlock(&epever_tracer6415an_lock);
}

int rs485_epever_tracer6415an_real_time_data(epever_tracer6415an_real_time_data_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  tracer6415an_real_time_data_cb = NULL;
  if (sys_try_lock(&epever_tracer6415an_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

    tracer6415an_real_time_data_cb = callback;
    epever_tracer6415an_err = MASTER_READ_INPUT_REGISTERS(EPEVER_TRACER6414AN_SLAVE_ADDRESS, EP_TRACER6415AN_REAL_TIME_DATA_INITIAL_ADDRESS, EP_TRACER6415AN_REAL_TIME_DATA_ELEMENTS, EPEVER_TRACER6414AN_TIMEOUT, rs485_epever_tracer6415an_real_time_data_receive); 

    if (epever_tracer6415an_err) {
      tracer6415an_real_time_data_cb = NULL;
      sys_unlock(&epever_tracer6415an_lock);
    }

  } else
    epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_REAL_TIME_DATA_BUSY;

  return epever_tracer6415an_err;
}

static EP_TRACER6415AN_REAL_TIME_STATUS epever_tracer6415an_real_time_status_record =  {0};
static epever_tracer6415an_real_time_status_cb tracer6415an_real_time_status_cb = NULL;

#define TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE(dest) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_real_time_status_record, dest, sizeof(epever_tracer6415an_real_time_status_record.dest))

#define TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE_U16(dest, n) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_real_time_status_record, dest, (sizeof(uint16_t)*(n)))

#define TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE_FINISH(dest) \
  TRACER6415AN_COPY_AND_FINISH(epever_tracer6415an_real_time_status_record, dest)

static void rs485_epever_tracer6415an_real_time_status_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  if ((epever_tracer6415an_err = status) == MASTER_TRANSFER_SUCCESS) {
      if (EP_TRACER6415AN_REAL_TIME_STATUS_ELEMENTS == data_size) { // Is redundant. ModBus checker guarantees that element has same size

        TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE(battery_status)
        TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE(charging_equipment_status)
        TRACER6415AN_REAL_TIME_STATUS_COPY_AND_ADVANCE_FINISH(discharging_equipment_status)

        epever_tracer6415an_err = 0; // Success
      } else
        epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_REAL_TIME_STATUS_ELEM_NOT_MATCH;
  }

  tracer6415an_real_time_status_cb(&epever_tracer6415an_err, &epever_tracer6415an_real_time_status_record);
  tracer6415an_real_time_status_cb = NULL;
  sys_unlock(&epever_tracer6415an_lock);
}

int rs485_epever_tracer6415an_real_time_status(epever_tracer6415an_real_time_status_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  tracer6415an_real_time_status_cb = NULL;
  if (sys_try_lock(&epever_tracer6415an_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

    tracer6415an_real_time_status_cb = callback;
    epever_tracer6415an_err = MASTER_READ_INPUT_REGISTERS(EPEVER_TRACER6414AN_SLAVE_ADDRESS, EP_TRACER6415AN_REAL_TIME_STATUS_INITIAL_ADDRESS, EP_TRACER6415AN_REAL_TIME_STATUS_ELEMENTS, EPEVER_TRACER6414AN_TIMEOUT, rs485_epever_tracer6415an_real_time_status_receive); 

    if (epever_tracer6415an_err) {
      tracer6415an_real_time_status_cb = NULL;
      sys_unlock(&epever_tracer6415an_lock);
    }

  } else
    epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_REAL_TIME_STATUS_BUSY;

  return epever_tracer6415an_err;
}

static EP_TRACER6415AN_STATISTICAL_PARAMETERS epever_tracer6415an_statistical_parameters_record =  {0};
static epever_tracer6415an_statistical_parameters_cb tracer6415an_statistical_parameters_cb = NULL;

#define TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(dest) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_statistical_parameters_record, dest, sizeof(epever_tracer6415an_statistical_parameters_record.dest))

#define TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE_U16(dest, n) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_statistical_parameters_record, dest, (sizeof(uint16_t)*(n)))

#define TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE_FINISH(dest) \
  TRACER6415AN_COPY_AND_FINISH(epever_tracer6415an_statistical_parameters_record, dest)

static void rs485_epever_tracer6415an_statistical_parameters_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  if ((epever_tracer6415an_err = status) == MASTER_TRANSFER_SUCCESS) {
      if (EP_TRACER6415AN_REAL_TIME_STATUS_ELEMENTS == data_size) { // Is redundant. ModBus checker guarantees that element has same size

        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(maximum_pv_voltage_today)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(minimum_pv_voltage_today)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(maximum_battery_voltage_today)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(minimum_battery_voltage_today)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(consumed_energy_today)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(consumed_energy_this_month)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(consumed_energy_this_year)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(total_consumed_energy)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(generated_energy_today);
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(generated_energy_this_month)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE(generated_energy_this_year)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE_U16(battery_voltage, 0x331A - 0x3313)
        TRACER6415AN_STATISTICAL_PARAMETERS_COPY_AND_ADVANCE_FINISH(battery_current)

        epever_tracer6415an_err = 0; // Success
      } else
        epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_STATISTICAL_PARAMETERS_ELEM_NOT_MATCH;
  }

  tracer6415an_statistical_parameters_cb(&epever_tracer6415an_err, &epever_tracer6415an_statistical_parameters_record);
  tracer6415an_statistical_parameters_cb = NULL;
  sys_unlock(&epever_tracer6415an_lock);
}

int rs485_epever_tracer6415an_statistical_parameters(epever_tracer6415an_statistical_parameters_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  tracer6415an_statistical_parameters_cb = NULL;
  if (sys_try_lock(&epever_tracer6415an_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

    tracer6415an_statistical_parameters_cb = callback;
    epever_tracer6415an_err = MASTER_READ_INPUT_REGISTERS(EPEVER_TRACER6414AN_SLAVE_ADDRESS, EP_TRACER6415AN_STATISTICAL_PARAMETERS_INITIAL_ADDRESS, EP_TRACER6415AN_STATISTICAL_PARAMETERS_ELEMENTS, EPEVER_TRACER6414AN_TIMEOUT, rs485_epever_tracer6415an_statistical_parameters_receive); 

    if (epever_tracer6415an_err) {
      tracer6415an_statistical_parameters_cb = NULL;
      sys_unlock(&epever_tracer6415an_lock);
    }

  } else
    epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_STATISTICAL_PARAMETERS_BUSY;

  return epever_tracer6415an_err;
}

static EP_TRACER6415AN_SETTING_PARAMETERS epever_tracer6415an_setting_parameters_record =  {0};
//
static epever_tracer6415an_setting_parameters_cb tracer6415an_setting_parameters_cb = NULL;

#define TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(dest) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_setting_parameters_record, dest, sizeof(epever_tracer6415an_setting_parameters_record.dest))

#define TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(dest, n) \
  TRACER6415AN_COPY_AND_ADVANCE_N(epever_tracer6415an_setting_parameters_record, dest, (sizeof(uint16_t)*(n)))

#define TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_FINISH(dest) \
  TRACER6415AN_COPY_AND_FINISH(epever_tracer6415an_setting_parameters_record, dest)

static void rs485_epever_tracer6415an_setting_parameters_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  (void)function;

  if ((epever_tracer6415an_err = status) == MASTER_TRANSFER_SUCCESS) {
      if (EP_TRACER6415AN_SETTING_PARAMETERS_ELEMENTS == data_size) { // Is redundant. ModBus checker guarantees that element has same size

        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(battey_type)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(battery_capacity)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(temperature_compensation_coeficient)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(high_volt_disconnect)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(changing_limit_voltage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(over_voltage_reconnect)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(equalization_voltage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(boost_voltage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(float_voltage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(boost_reconnect_voltage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(low_voltage_reconnect)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(under_voltage_warning)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(low_voltage_disconnect)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(dischanging_limit_voltage, 0x9013 - 0x900E)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(real_time_clock_A)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(real_time_clock_B)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(battery_temperature_warning_upper_limit)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(battery_temperature_warning_lower_limit)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(controller_inner_temperature_upper_limit)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(controller_inner_temperature_lower_limit)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(day_time_threshold_vold)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(light_signal_startup_night_delay_time)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(light_time_threshold_volt)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(light_signal_close_day_delay_time, 0x903D - 0x9021)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(load_controlling_nodes)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(working_time_lenght_1)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(working_time_lenght_2, 0x9042 - 0x903F)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_1_A)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_1_B)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_1_C)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_off_timer_1_A)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_off_timer_1_B)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_off_timer_1_C)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_2_A)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_2_B)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_on_timer_2_C)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_off_timer_2_A)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(turn_off_timer_2_B)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(turn_off_timer_2_C, 0x9063 - 0x904D)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(backlight_time, 0x9065 - 0x9063)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(lenght_of_time)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(device_configure_of_main_power_supply)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE_U16(battery_rated_voltage_code, 0x906A - 0x9067)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(default_load_on_off_in_manual_mode)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(equalize_duration)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(boost_duration)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(discharge_percentage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(charging_percentage)
        TRACER6415AN_SETTING_PARAMETERS_COPY_AND_ADVANCE(management_modes_of_battery_charging_and_discharging)

        epever_tracer6415an_err = 0; // Success
      } else
        epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_SETTING_PARAMETERS_ELEM_NOT_MATCH;
  }

  tracer6415an_setting_parameters_cb(&epever_tracer6415an_err, &epever_tracer6415an_setting_parameters_record);
  tracer6415an_setting_parameters_cb = NULL;
  sys_unlock(&epever_tracer6415an_lock);
}

int rs485_epever_tracer6415an_setting_parameters(epever_tracer6415an_setting_parameters_cb callback, uint32_t wait_unlock_timeout)
{
  TIMEOUT_MS timeout_ms;

  tracer6415an_setting_parameters_cb = NULL;
  if (sys_try_lock(&epever_tracer6415an_lock, &timeout_ms, wait_unlock_timeout, NULL)) {

    tracer6415an_setting_parameters_cb = callback;
    epever_tracer6415an_err = MASTER_READ_INPUT_REGISTERS(EPEVER_TRACER6414AN_SLAVE_ADDRESS, EP_TRACER6415AN_SETTING_PARAMETERS_INITIAL_ADDRESS, EP_TRACER6415AN_SETTING_PARAMETERS_ELEMENTS, EPEVER_TRACER6414AN_TIMEOUT, rs485_epever_tracer6415an_setting_parameters_receive); 

    if (epever_tracer6415an_err) {
      tracer6415an_setting_parameters_cb = NULL;
      sys_unlock(&epever_tracer6415an_lock);
    }

  } else
    epever_tracer6415an_err = E_EPEVER_TRACER_6415AN_SETTING_PARAMETERS_BUSY;

  return epever_tracer6415an_err;
}
