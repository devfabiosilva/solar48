#ifndef SOLAR48_CONFIG_H
 #define SOLAR48_CONFIG_H

// **** Clock and SysTick system clocks ****

#define SYS_TICK_FREQ_HZ 1000UL  // in ticks per seconds
#define CPU_FREQ_HZ      72000000UL // System core frequency in Hertz

#define SYS_TICK_DIV 8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk or 1 if no prescale systick timer (enable SysTick_CTRL_CLKSOURCE_Msk)
#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)

_Static_assert(SYSTICK_TICKS <= 0xFFFFFF, "SYSTICK_TICKS too high for SysTick LOAD register");

#define PCLK2 = CPU_FREQ_HZ
#define PCLK1 = CPU_FREQ_HZ / 2

// *****************************************

// ****      USB CDC configuration      ****
/* Define size for the receive and transmit buffer over CDC */
#define APP_RX_DATA_SIZE  256
#define APP_TX_DATA_SIZE  256

#define USB_TRANSMIT_TIMEOUT_MS 10
#define USB_RECEIVE_TIMEOUT_MS 16

#define USBD_MAX_STR_DESC_SIZ    128 //512
#define USB_SIZ_STRING_SERIAL    0x1A

#define 	UID_BASE            0x1FFFF7E8UL    //!< Unique device ID register base address
#define         DEVICE_ID1          (UID_BASE)
#define         DEVICE_ID2          (UID_BASE + 0x4)
#define         DEVICE_ID3          (UID_BASE + 0x8)

#define USBD_VID                       1155
#define USBD_LANGID_STRING             1033
#define USBD_MANUFACTURER_STRING       "DevFabioSilva"
#define USBD_PID_FS                    22336
#define USBD_PRODUCT_STRING_FS         "Solar48 Virtual Communication Port for 48 V solar panel"
#define USBD_CONFIGURATION_STRING_FS   "CDC Config"
#define USBD_INTERFACE_STRING_FS       "CDC Interface"

// *****************************************

// ****        PANIC HANDLER            ****
//#define USE_USB_PRINTF_ON_PANIC // Uncomment to enable usb panic log. Default: oled screen
// *****************************************

// ****      PCLK1 FREQ IN MHZ          ****
#define PCLK1_FREQ_IN_MHZ 36
// *****************************************

// ****     UART configuration         ****
#define UART1_TX_RX_BUF 32

// *****************************************

// ****   OLED BUFFER configuration     ****
#define OLED_BUFFER_TIMEOUT_MS 10
#define OLED_BUFFER_TRANSFER_TIMEOUT_MS 4*OLED_BUFFER_TIMEOUT_MS
#define OLED_INITIALIZE_TIME_MS 100
#define OLED_OLED_LOCK_TIMEOUT_MS 120

#define OLED_BUF_MAX_SIZE 64

// *****************************************

// ****   OLED PRINTF configuration     ****
#define OLED_PRINTIF_TIMEOUT_MS 5
#define OLED_PRINTIF_QUEUE_SIZE 16 // -1 is loaded default
#define OLED_PRINTF_ENABLE_GLOBAL_ERROR true

// *****************************************

// ****     STDOUT configuration        ****
#define STDOUT_TIMEOUT_MS 50

// *****************************************

// ****         SYS QUEUE SIZE          ****
#define DEFAULT_SYS_QUEUE_SIZE 4
#define SYS_QUEUE_TIMEOUT_MS 10
#define SYS_QUEUE_INIT_TIMEOUT_MS 50
#define SYS_QUEUE_TRY_RUN_TIMEOUT_MS 10*SYS_QUEUE_TIMEOUT_MS
#define SYS_QUEUE_DEINIT_TIMEOUT_MS 3000
#define SYS_QUEUE_TRY_UNLOCK_GLOBAL_TIMEOUT_MS 1000
#define SYS_QUEUE_DESTROY_TIMEOUT_MS 2000
_Static_assert(SYS_QUEUE_DEINIT_TIMEOUT_MS > SYS_QUEUE_DESTROY_TIMEOUT_MS, "SYS_QUEUE_DEINIT_TIMEOUT_MS should be > SYS_QUEUE_DESTROY_TIMEOUT_MS");

// *****************************************

// ****   MASTER RS485 RTU CONFIG       ****
#define IMPLEMENT_RS485_MASTER_OVER_UART1
#define IMPLEMENT_RS485_SLAVE_OVER_UART2

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
 #define MASTER_RS485_TIMEOUT_MS 50
#endif

// *****************************************

// ****      ERROR LOG ERRORS           ****
#define USE_USB_PRINTF_IN_ERROR_LOGS
#define MAX_ERROR_LOGS_INSTANCES 6
// *****************************************

#ifdef RTOS_SOLAR48
 #include <FreeRTOS/FreeRTOSConfig.h>
/*
 #define LED_BLINK_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_INST_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_INT_EXT_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_INT_INT_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_PERIPH_TIME_EV_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_SYS_QUEUE_STACK_SIZE 2*configMINIMAL_STACK_SIZE
 #define PROCESS_RTOS_MILLI_STACK_SIZE 1*configMINIMAL_STACK_SIZE
*/

 #define LED_BLINK_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_INST_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_INT_EXT_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_INT_INT_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_PERIPH_TIME_EV_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_SYS_QUEUE_STACK_SIZE 3*configMINIMAL_STACK_SIZE
 #define PROCESS_RTOS_MILLI_STACK_SIZE 1*configMINIMAL_STACK_SIZE

#endif

#endif

