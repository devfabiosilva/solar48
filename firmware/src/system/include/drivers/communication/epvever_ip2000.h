#ifndef EPEVER_IP2000_H
 #define EPEVER_IP2000_H

#include <stdint.h>

#define EPEVER_IP2000_SLAVE_ADDRESS (uint8_t)3
#define EPEVER_IP2000_TIMEOUT 4

//Coil (Read|Write)
#define CLEAR_THE_FAULTS 0x0013
#define LOCAL_OR_REMOTE 0x0011
#define INVERTER_ON_OFF 0x000F
#define POWER_SAVING_MODE_ENABLE 0x0004

// Holding register: (Read and write, function code 0x03 and 0x10)
#define LOW_INPUT_VOLTAGE 0x902F
#define LOW_INPUT_VOLTAGE_5S 0x9030
#define LOW_INPUT_VOLTAGE_RECOVERY_VOLTAGE 0x9031
#define HIGH_INPUT_VOLTAGE_RECOVERY_VOLTAGE 0x9032
#define HIGH_INPUT_VOLTAGE_5S 0x9033
#define HIGH_INPUT_VOLTAGE 0x9034
#define HIGH_INPUT_CORRENT 0x9035
#define HIGH_INPUT_CORRENT_RECOVERY_VOLTAGE 0x9036

// Note: The above eight variables’ addresses need to be sent at one time. 
// For the variables that cannot be modified, you can fill in the default value or 0.

//Note: Only the IP-Plus series, NP4000-22, and NP5000-42, support the Output AC voltage setting and Output AC frequency setting. IP series and other NPower models adopt the hardware dial switches.

#define OUTPUT_AC_VOLTAGE_SETTING 0x9022
#define OUTPUT_AC_FREQUENCY_SETTING 0x9023

//Input register: ( Read only code0x04)

#define LOAD_INPUT_VOLTAGE 0x3108
#define LOAD_INPUT_CURRENT 0x3109
#define LOAD_INPUT_POWER_L 0x310A
#define LOAD_INPUT_POWER_H 0x310B
#define LOAD_OUTPUT_VOLTAGE 0x310C
#define LOAD_OUTPUT_CURRENT 0x310D
#define LOAD_OUTPUT_POWER_L 0x310E
#define LOAD_OUTPUT_POWER_H 0x310F
#define LOAD_OUTPUT_RESERVED_1 0x3110
#define LOAD_TEMPERATURE 0x3111
#define HEAT_SINK_TEMPERATURE 0x3112
#define LOAD_STATUS 0x3202
 #define VOLTAGE_STATUS(x) (x<<14)
 #define STATUS_NORMAL_INPUT_VOLTAGE VOLTAGE_STATUS(0b00)
 #define STATUS_LOW_INPUT_VOLTAGE VOLTAGE_STATUS(0b01)
 #define STATUS_HIGTH_INPUT_VOLTAGE VOLTAGE_STATUS(0b10)
 #define STATUS_NO_CONNECT_O_INPUT_POWER VOLTAGE_STATUS(0b11)

 #define STATUS_OUTPUT_POWER(x) (x<<12)
 #define STATUS_LIGHT_LOAD STATUS_OUTPUT_POWER(0xb00)
 #define STATUS_MEDIUM_LOAD STATUS_OUTPUT_POWER(0xb01)
 #define STATUS_NOMINAL_LOAD STATUS_OUTPUT_POWER(0xb10)
 #define STATUS_OVERLOAD STATUS_OUTPUT_POWER(0xb11)

 #define STATUS_OUTPUT_FAIL (1<<5)
 #define STATUS_HIGH_VOLTAGE_SHORT_CIRCUIT (1<<6)
 #define STATUS_INPUT_OVER_CURRENT (1<<7)
 #define STATUS_ABNORMAL_OUTPUT_VOLTAGE (1<<8)
 #define STATUS_UNABLE_TO_STOP_DISCHARGE (1<<9)
 #define STATUS_UNABLE_TO_DISCHARGE (1<<10)
 #define STATUS_SHORT_CIRCUIT (1<<11)
 #define STATUS_RUN (1<<1)
 #define STATUS_STANDBY (0<<1)
 #define STATUS_NORMAL (0<<0)
 #define STATUS_FAULTS (1<<0)

//Discrete register: ( Read only, function code0x02)
#define DEVICE_OVER_TEMPERATURE 0x2000

typedef struct ep_ip2000_t {
  int16_t input_voltage;
  int16_t input_current;
  int32_t input_power;
  int16_t output_voltage;
  int16_t output_current;
  int32_t output_power;
  uint16_t reserved;
  int16_t temp;
  int16_t heat_sink_temp;
//  uint16_t status;
//  uint16_t OVER_TEMP;
} EP_IP2000;
#define EP_IP2000_ELEM_SIZE (sizeof(EP_IP2000)/sizeof(uint16_t))
_Static_assert(EP_IP2000_ELEM_SIZE == 11, "Number of elements in EP_IP2000 must MATCH from range 0x3108 - 0x3112");
_Static_assert(sizeof(EP_IP2000) == 22, "Wrong packet memory block size of EP_IP2000");

typedef void (*ep_ip2000cb)(int *, EP_IP2000 *);
typedef void (*ep_ip2000status_cb)(int *, uint16_t *);
typedef void (*ep_ip2000device_over_temp_cb)(int *, uint16_t *);
typedef void (*ep_ip2000coils_read_write_cb)(int *, uint16_t *);

int read_ep2000(ep_ip2000cb, uint32_t);
int read_ep2000_status(ep_ip2000status_cb, uint32_t);
int read_ep2000_over_temperature(ep_ip2000device_over_temp_cb, uint32_t);

//read coils
#define read_ep2000_clear_faults(callback, timeout) read_ep2000_read_coil(CLEAR_THE_FAULTS, callback, timeout)
#define read_ep2000_local_or_remote(callback, timeout) read_ep2000_read_coil(LOCAL_OR_REMOTE, callback, timeout)
#define read_ep2000_inverter_on_off(callback, timeout) read_ep2000_read_coil(INVERTER_ON_OFF, callback, timeout)
#define read_ep2000_pwr_saving_mode_enabled(callback, timeout) read_ep2000_read_coil(POWER_SAVING_MODE_ENABLE, callback, timeout)

//write coils
#define write_ep2000_clear_faults(val, callback, timeout) read_ep2000_write_coil(CLEAR_THE_FAULTS, val, callback, timeout)
#define write_ep2000_local_or_remote(val, callback, timeout) read_ep2000_write_coil(LOCAL_OR_REMOTE, val, callback, timeout)
#define write_ep2000_inverter_on_off(val, callback, timeout) read_ep2000_write_coil(INVERTER_ON_OFF, val, callback, timeout)
#define write_ep2000_pwr_saving_mode_enabled(val, callback, timeout) read_ep2000_write_coil(POWER_SAVING_MODE_ENABLE, val, callback, timeout)


#endif

