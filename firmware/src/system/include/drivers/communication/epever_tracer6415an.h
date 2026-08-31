#ifndef EPEVER_TRACER_6415AN_H
 #define EPEVER_TRACER_6415AN_H

#include <stdint.h>

#define EPEVER_TRACER6414AN_SLAVE_ADDRESS (uint8_t)0
#define EPEVER_TRACER6414AN_TIMEOUT 4

//Rated Datum (Read Only)
typedef struct ep_tracer6415an_rated_datum_t {
  uint16_t array_rated_votage;
  uint16_t array_rated_current;
  uint32_t array_rated_power;
  uint16_t battery_rated_voltage;
  uint16_t battery_rated_current;
  uint32_t battery_rated_power;
  uint16_t charging_mode; // 00 Connect/disconnect, 01 PWM, 02 MPPT
  uint16_t rated_current_load;
} EP_TRACER6415AN_RATED_DATUM;
#define RATED_DATUM_INITIAL_ADDRESS 0x3000
#define RATED_DATUM_END_ADDRESS 0x300E
_Static_assert(RATED_DATUM_END_ADDRESS > RATED_DATUM_INITIAL_ADDRESS, "Wrong start/end RATED_DATUM address range");
#define EP_TRACER6415AN_RATED_DATUM_ELEMENTS (RATED_DATUM_END_ADDRESS - RATED_DATUM_INITIAL_ADDRESS) // 14 = from 0x3000 to 0x300E

typedef void (*epever_tracer6415an_read_rated_datum_cb)(int *, EP_TRACER6415AN_RATED_DATUM *);

int rs485_epever_tracer6415an_read_rated_datum(epever_tracer6415an_read_rated_datum_cb, uint32_t);

//Real-time Datum (Read Only)
typedef struct ep_tracer6415an_real_time_data_t {
  uint16_t pv_array_input_voltage;
  uint16_t pv_array_input_current;
  uint32_t pv_array_input_power;
  uint32_t battery_power;
  uint16_t load_voltage;
  uint16_t load_current;
  uint32_t load_power;
  uint16_t battery_temperature;
  uint16_t temperature_inside_equipament;
  uint16_t battery_soc;
  uint16_t remote_battery_temperature;
  uint16_t battery_real_rated_power;
} EP_TRACER6415AN_REAL_TIME_DATA;
#define EP_TRACER6415AN_REAL_TIME_DATA_INITIAL_ADDRESS 0x3100
#define EP_TRACER6415AN_REAL_TIME_DATA_END_ADDRESS 0x311D
_Static_assert(EP_TRACER6415AN_REAL_TIME_DATA_END_ADDRESS > EP_TRACER6415AN_REAL_TIME_DATA_INITIAL_ADDRESS, "Wrong start/end REAL_TIME_DATA address range");
#define EP_TRACER6415AN_REAL_TIME_DATA_ELEMENTS (EP_TRACER6415AN_REAL_TIME_DATA_END_ADDRESS - EP_TRACER6415AN_REAL_TIME_DATA_INITIAL_ADDRESS)

typedef void (*epever_tracer6415an_real_time_data_cb)(int *, EP_TRACER6415AN_REAL_TIME_DATA *);
int rs485_epever_tracer6415an_real_time_data(epever_tracer6415an_real_time_data_cb, uint32_t);

//Real-time Status (Read Only)
typedef struct ep_tracer6415an_real_time_status_t {
  uint16_t battery_status;
  uint16_t charging_equipment_status;
  uint16_t discharging_equipment_status;
} EP_TRACER6415AN_REAL_TIME_STATUS;
#define EP_TRACER6415AN_REAL_TIME_STATUS_INITIAL_ADDRESS 0x3200
#define EP_TRACER6415AN_REAL_TIME_STATUS_END_ADDRESS 0x3202
_Static_assert(EP_TRACER6415AN_REAL_TIME_DATA_END_ADDRESS > EP_TRACER6415AN_REAL_TIME_DATA_INITIAL_ADDRESS, "Wrong start/end REAL_TIME_STATUS address range");
#define EP_TRACER6415AN_REAL_TIME_STATUS_ELEMENTS (EP_TRACER6415AN_REAL_TIME_STATUS_END_ADDRESS - EP_TRACER6415AN_REAL_TIME_STATUS_INITIAL_ADDRESS)

typedef void (*epever_tracer6415an_real_time_status_cb)(int *, EP_TRACER6415AN_REAL_TIME_STATUS *);
int rs485_epever_tracer6415an_real_time_status(epever_tracer6415an_real_time_status_cb, uint32_t);

//Statistical Parameters (Read Only)
typedef struct ep_tracer6415an_statistical_parameters_t {
  uint16_t maximum_pv_voltage_today;
  uint16_t minimum_pv_voltage_today;
  uint16_t maximum_battery_voltage_today;
  uint16_t minimum_battery_voltage_today;
  uint32_t consumed_energy_today;
  uint32_t consumed_energy_this_month;
  uint32_t consumed_energy_this_year;
  uint32_t total_consumed_energy;
  uint32_t generated_energy_today;
  uint32_t generated_energy_this_month;
  uint32_t generated_energy_this_year;
  uint16_t battery_voltage;
  uint32_t battery_current;
} EP_TRACER6415AN_STATISTICAL_PARAMETERS;
#define EP_TRACER6415AN_STATISTICAL_PARAMETERS_INITIAL_ADDRESS 0x3300
#define EP_TRACER6415AN_STATISTICAL_PARAMETERS_END_ADDRESS 0x331C
_Static_assert(EP_TRACER6415AN_STATISTICAL_PARAMETERS_END_ADDRESS > EP_TRACER6415AN_STATISTICAL_PARAMETERS_INITIAL_ADDRESS, "Wrong start/end STATISTICAL_PARAMETERS address range");
#define EP_TRACER6415AN_STATISTICAL_PARAMETERS_ELEMENTS (EP_TRACER6415AN_STATISTICAL_PARAMETERS_END_ADDRESS - EP_TRACER6415AN_STATISTICAL_PARAMETERS_INITIAL_ADDRESS)

typedef void (*epever_tracer6415an_statistical_parameters_cb)(int *, EP_TRACER6415AN_STATISTICAL_PARAMETERS *);
int rs485_epever_tracer6415an_statistical_parameters(epever_tracer6415an_statistical_parameters_cb, uint32_t);

//Setting Parameter (Read and Write)
//Read Holding Register (0x03) and Write Multiple Holding Register (0x10)
typedef struct ep_tracer6415an_setting_parameters_t {
  uint16_t battey_type;
  uint16_t battery_capacity;
  uint16_t temperature_compensation_coeficient;
  uint16_t high_volt_disconnect;
  uint16_t changing_limit_voltage;
  uint16_t over_voltage_reconnect;
  uint16_t equalization_voltage;
  uint16_t boost_voltage;
  uint16_t float_voltage;
  uint16_t boost_reconnect_voltage;
  uint16_t low_voltage_reconnect;
  uint16_t under_voltage_warning;
  uint16_t low_voltage_disconnect;
  uint16_t dischanging_limit_voltage;
  uint32_t real_time_clock_A;
  uint32_t real_time_clock_B;
  uint16_t battery_temperature_warning_upper_limit;
  uint16_t battery_temperature_warning_lower_limit;
  uint16_t controller_inner_temperature_upper_limit;
  uint16_t controller_inner_temperature_lower_limit;
  uint16_t day_time_threshold_vold;
  uint16_t light_signal_startup_night_delay_time;
  uint16_t light_time_threshold_volt;
  uint16_t light_signal_close_day_delay_time;
  uint16_t load_controlling_nodes;
  uint16_t working_time_lenght_1;
  uint16_t working_time_lenght_2;
  uint16_t turn_on_timer_1_A;
  uint16_t turn_on_timer_1_B;
  uint16_t turn_on_timer_1_C;
  uint16_t turn_off_timer_1_A;
  uint16_t turn_off_timer_1_B;
  uint16_t turn_off_timer_1_C;
  uint16_t turn_on_timer_2_A;
  uint16_t turn_on_timer_2_B;
  uint16_t turn_on_timer_2_C;
  uint16_t turn_off_timer_2_A;
  uint16_t turn_off_timer_2_B;
  uint16_t turn_off_timer_2_C;
  uint16_t backlight_time;
  uint16_t lenght_of_time;
  uint16_t device_configure_of_main_power_supply;
  uint16_t battery_rated_voltage_code;
  uint16_t default_load_on_off_in_manual_mode;
  uint16_t equalize_duration;
  uint16_t boost_duration;
  uint16_t discharge_percentage;
  uint16_t charging_percentage;
  uint16_t management_modes_of_battery_charging_and_discharging;
} EP_TRACER6415AN_SETTING_PARAMETERS;
#define EP_TRACER6415AN_SETTING_PARAMETERS_INITIAL_ADDRESS 0x9000
#define EP_TRACER6415AN_SETTING_PARAMETERS_END_ADDRESS 0x9070
_Static_assert(EP_TRACER6415AN_SETTING_PARAMETERS_END_ADDRESS > EP_TRACER6415AN_SETTING_PARAMETERS_INITIAL_ADDRESS, "Wrong start/end SETTING_PARAMETERS address range");
#define EP_TRACER6415AN_SETTING_PARAMETERS_ELEMENTS (EP_TRACER6415AN_SETTING_PARAMETERS_END_ADDRESS - EP_TRACER6415AN_SETTING_PARAMETERS_INITIAL_ADDRESS)

typedef void (*epever_tracer6415an_statistical_parameters_cb)(int *, EP_TRACER6415AN_STATISTICAL_PARAMETERS *);
int rs485_epever_tracer6415an_statistical_parameters(epever_tracer6415an_statistical_parameters_cb, uint32_t);

typedef void (*epever_tracer6415an_setting_parameters_cb)(int *, EP_TRACER6415AN_SETTING_PARAMETERS *);

#endif

