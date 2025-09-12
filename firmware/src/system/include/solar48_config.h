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

#endif

