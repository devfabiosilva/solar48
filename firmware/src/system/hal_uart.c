#include <registers.h>
#include <sys_interrupts.h>
#include <dma.h>
#include <solar48_config.h>
#include <time.h>
#include <system.h>
#include <hal_uart.h>
//TODO understand this behaviour https://community.st.com/t5/stm32-mcus-products/overrun-error-in-lpuart-when-using-hardware-flow-control/td-p/61086
#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1

#include <rs485.h>
#include <memory.h>

extern SOLAR48_RS485_RTU master_rs485_rtu;
extern uint8_t modbus_master_buffer[];
extern void _master_receive(int);

static void uart1_receive(
  uint8_t *, size_t,
  uart_callback_func,
  uint32_t
);

static const struct master_rs485_speed_t {
  uint32_t uart1_speed;
  uint16_t tim2_adj;
} MASTER_RS485_SPEED[] = {
    {UART1_2_4_KBPS, 952},
    {UART1_9_6_KBPS, 476},
    {UART1_19_2_KBPS, 336},
    {UART1_57_6_KBPS, 194},
    {UART1_115_2_KBPS, 137},
    {UART1_230_769_KBPS, 97},
    {UART1_461_538_KBPS, 68},
    {UART1_923_076_KBPS, 47},
    {UART1_2250_KBPS, 31},
    {UART1_4500_KBPS, 22}
};

#endif

struct uart_control_t {
  volatile bool start_monitore;               // UART start timeout monitore event
  volatile bool locked;                       // UART locked
  volatile TIMEOUT_MS timeout;                // UART timeout
  volatile uint32_t status_register;          // UART status register
  uart_callback_func uart_callback;           // Uart transmit/receive callback event
};

volatile struct uart_control_t uart1_control = {0};

// Solar48 uses DMA1 Channel4 for TX and DMA1 Channel5 to Receive UART1 data.
// This layer is used in RS485 (1) in master mode
// 13 Direct memory access controller (DMA) Page 274

// --- Table 78. Summary of DMA1 requests for each channel Pag 282 ---
// DMA1 for UART1 Tx events IRQ
void DMA1_Channel4_IRQHandler()
{
  uint32_t dma1_ch4_sr = DMA1_ISR;

  // Clear Channel 4 global interrupts status registers
  DMA1_IFCR = (CTEIF4|CHTIF4|CTCIF4|CGIF4);

  DMA1_CCR4 &= ~(DMA1_CCR4_EN); // Disable DMA1_Channel4

  // DMA1 error on transfer
  if (dma1_ch4_sr & TEIF4) {
    __atomic_store_n(&uart1_control.status_register, E_UART1_DMA1_CH4_TRANSMIT_ERROR, __ATOMIC_RELEASE);
    return;
  }

  // Transfer complete
  if (dma1_ch4_sr & TCIF4) {

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
    TIM2_SR &= ~(UIF); // Disable overflow flag
    TIM2_CNT = 0;
    TIM2_CR1 |= CEN; // Enable Timer 2
#endif

   __atomic_store_n(&uart1_control.status_register, UART1_TRANSFER_COMPLETE, __ATOMIC_RELEASE);

  }
}

/*
// DMA1 for UART1 Rx events IRQ
void DMA1_Channel5_IRQHandler()
{
  uint32_t dma1_ch5_sr = DMA1_ISR;

  // Clear Channel 4 global interrupts status registers
  DMA1_IFCR = (CTEIF5|CHTIF5|CTCIF5|CGIF5);

  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5

  // DMA1 error on receive
  if (dma1_ch5_sr & TEIF5) {
    __atomic_store_n(&uart1_control.status_register, E_UART1_DMA1_CH5_RECEIVE_ERROR, __ATOMIC_RELEASE);
    return;
  }

  // Receive complete
  if (dma1_ch5_sr & TCIF5) {
#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
    if (master_rs485_rtu.first_pass) {

      DMA1_CMAR5 = (uint32_t)master_rs485_rtu.second_pass; // Memory address Page 288

      if (read_uint8(master_rs485_rtu.first_pass) < 0x81) {

        DMA1_CNDTR5 = (uint16_t)master_rs485_rtu.second_pass_len; // Memory size

        if (master_rs485_rtu.transfer_left_data_limit == 0)
          master_rs485_rtu.second_pass = NULL;

      } else {

        DMA1_CNDTR5 = 3; // 1 (exception code) +  2 (crc)
        master_rs485_rtu.second_pass = NULL;

      }

      master_rs485_rtu.first_pass = NULL;
      DMA1_CCR5 |= (DMA1_CCR5_EN); // Enable again to second pass or exception

    } else if (master_rs485_rtu.second_pass) {

      DMA1_CMAR5 = (uint32_t)&master_rs485_rtu.second_pass[1]; // Memory address Page 288

      uint8_t data_sz = read_uint8(master_rs485_rtu.second_pass);

      if (data_sz > master_rs485_rtu.transfer_left_data_limit)
        data_sz = master_rs485_rtu.transfer_left_data_limit; // Guard: Security

      DMA1_CNDTR5 = (uint16_t)(data_sz + 2); // rest of the data plus crc (2 bytes)

      master_rs485_rtu.second_pass = NULL;
      DMA1_CCR5 |= (DMA1_CCR5_EN); // Enable again to second pass or exception

    } else
#endif
    __atomic_store_n(&uart1_control.status_register, UART1_RECEIVE_COMPLETE, __ATOMIC_RELEASE);

  }

}
*/
// DMA1 for UART1 Rx events IRQ
void DMA1_Channel5_IRQHandler()
{
  uint32_t dma1_ch5_sr = DMA1_ISR;

  // Clear Channel 4 global interrupts status registers
  DMA1_IFCR = (CTEIF5|CHTIF5|CTCIF5|CGIF5);

  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5

  // DMA1 error on receive
  if (dma1_ch5_sr & TEIF5) {
    __atomic_store_n(&uart1_control.status_register, E_UART1_DMA1_CH5_RECEIVE_ERROR, __ATOMIC_RELEASE);
    return;
  }

  // Receive complete
  if (dma1_ch5_sr & TCIF5) {
#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
    __atomic_store_n(&uart1_control.status_register, E_RS485_MASTER_DMA5_BUFFER_OVERFLOW, __ATOMIC_RELEASE);
#else
    __atomic_store_n(&uart1_control.status_register, UART1_RECEIVE_COMPLETE, __ATOMIC_RELEASE);
#endif
  }

}

//Table 196. USART interrupt requests page 816
/*
void USART1_IRQHandler()
{
  // Clear any status register
  uint32_t uart1_has_error = (USART1_SR & (ORE|NE|FE|PE));
  (void)USART1_DR;

  if (uart1_has_error) {
    USART1_CR1 &= ~(RE);  // Ensure Receive is disable
    DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5
    __atomic_store_n(&uart1_control.status_register, E_UART1_RECEIVE_ERROR_BASE | uart1_has_error, __ATOMIC_RELEASE);
  }
}
*/
void USART1_IRQHandler()
{
  // Clear any status register
  uint32_t status = USART1_SR;
  (void)USART1_DR;
  uint32_t uart1_has_error = (status & (ORE|NE|FE|PE));

  if (uart1_has_error) {
    USART1_CR1 &= ~(RE);  // Ensure Receive is disable
    DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5
    __atomic_store_n(&uart1_control.status_register, E_UART1_RECEIVE_ERROR_BASE | uart1_has_error, __ATOMIC_RELEASE);
    return;
  }

  if (status & IDLE)
    __atomic_store_n(&uart1_control.status_register, UART1_RECEIVE_COMPLETE, __ATOMIC_RELEASE);
}

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
/*
void TIM2_IRQHandler()
{
  if (TIM2_SR & UIF) {

    TIM2_SR &= ~(UIF);

    uart1_receive(modbus_master_buffer, (size_t)master_rs485_rtu.first_pass_len, _master_receive, master_rs485_rtu.timeout_ms);
  }
}
*/

void TIM2_IRQHandler()
{
  if (TIM2_SR & UIF) {

    TIM2_SR &= ~(UIF);

    uart1_receive(modbus_master_buffer, (size_t)MODBUS_MASTER_BUFFER_SIZE, _master_receive, master_rs485_rtu.timeout_ms);
  }
}
#endif

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
void init_uart1(enum uart1_speed_e speed, enum uart1_mode_e mode)
#else
void init_master_rs485(enum rs485_master_speed_e speed, enum rs485_master_mode_e mode)
#endif
{

  // 7.3.7 - APB2 peripheral clock enable register (RCC_APB2ENR) Page 112
  RCC_APB2ENR |= (IOPAEN|AFIOEN);   // Enables GPIOA. Page 113
  RCC_APB2ENR |= USART1EN; // Enables USART1. Page 113

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1

  RCC_APB1ENR |= TIM2EN; // Enable Timer 2 page 115

  // ---- Control pin (PA8) for MAX485 ----
  // PA8 -> Output pin for MAX485 DE/RE_B. Default 1
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b00); // Output mode, max speed 50 MHz and GPIO as output mode
//  GPIOA_CRH |= GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b10); // Output alternat function for PA8 and max speed 50Mhz
  MASTER_RS485_DRIVER_TRANSMIT_MODE // Enable RS485 Master transmit pin
#endif

  // 7.3.6 AHB peripheral clock enable register (RCC_AHBENR) Page 111
  RCC_AHBENR |= DMA1EN;

  // ---- Configure GPIOA TX ----
  // PA9 = TX (AF push-pull), PA10 = RX (input floating)
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE9_VAL(0b11) | GPIOA_CNF9_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE9_VAL(0b11) | GPIOA_CNF9_VAL(0b10); // Output mode, max speed 50 MHz and Alternate function output Push-pull

  // ---- Configure GPIOA RX ----
  // PA9 = TX (AF push-pull), PA10 = RX (input floating)
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE10_VAL(0b11) | GPIOA_CNF10_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE10_VAL(0b00) | GPIOA_CNF10_VAL(0b01); // Floating input mode (reset state)

  // Set UART 1 Speed
  //See page 798: 27.3.4 Fractional baud rate generation
#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
  USART1_BRR = MASTER_RS485_SPEED[speed].uart1_speed;
#else
  USART1_BRR = (uint32_t)speed;
#endif
//  USART_GTPR =  GT(16)|0b00001; // Divide source clock by 62 * GT(val)


  USART1_CR3 = (
                  DMAT | // DMA enable transmitter
                  DMAR | // DMA enable receiver
//                  SCEN|
                  EIE    // Error interrupt enable
               );

  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART1_CR1 = (
                 ((uint32_t)mode)|
                 /*PCE|M|*/  //Parit control enable and stop bit + 9 bit
                 /*RE|*/
                 /*PEIE|*/ // Parity error interrupt enabled
                 IDLEIE|
                 TE // Transmit enable
               );

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
  TIM2_CR1 = OPM; // One Pulse Mode activated for TIMER2 page 404
  TIM2_CNT = 0; // Reset counter page 418
  uint16_t tim2_adj = MASTER_RS485_SPEED[speed].tim2_adj;
  TIM2_PSC = tim2_adj;
  TIM2_ARR = tim2_adj - 1;
  TIM2_SR &= ~(UIF); // Reset overflow flag
  TIM2_DIER = UIE; // Timer 2 interrupt enable
#endif

  USART1_CR1 |= UE;      // Enable UART1

  dma1_channel4_init((void *)&USART1_DR);
  dma1_channel5_init((void *)&USART1_DR);

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
  __nvic_set_priority(TIM2_IRQn, TIM2_PRIO); // Set timer 2 priority
  __nvic_enable_irq(TIM2_IRQn); // Enable IRQ for TIMER2
#endif

  __nvic_set_priority(USART1_IRQn, UART1_PRIO);
  __nvic_enable_irq(USART1_IRQn);

}

inline bool uart1_is_busy()
{
  return (((DMA1_CCR4 & DMA1_CCR4_EN) != 0) || ((USART1_CR1 & RE) != 0));
}

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
static void
#else
enum uart1_status_t
#endif
uart1_receive(
  uint8_t *data, size_t data_size,
  uart_callback_func receive_uart_callback,
  uint32_t timeout
)
{
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
  if ((data != NULL) && (data_size > 0)) {

    if (uart1_is_busy())
      return UART_BUSY;

    TIMEOUT_MS timeout_ms;
    if (!sys_try_lock(&uart1_control.locked, &timeout_ms, 1, NULL)) // 1 milliseconds to wait
      return UART_LOCKED;
#endif

    // We need to use __atomic here because process_uart1_time_event is always running
    __atomic_store_n(&uart1_control.start_monitore, false, __ATOMIC_RELEASE);

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
    // Prepare MAX485 driver receive mode
    MASTER_RS485_DRIVER_RECEIVE_MODE
#endif

    USART1_CR1 &= ~(RE);           // Ensure Receive is disable
    DMA1_CCR5 &= ~(DMA1_CCR5_EN);  // Disable DMA1_Channel5
    DMA1_CCR4 &= ~(DMA1_CCR4_EN);  // Disable DMA1_Channel4

    //USART1_CR1 &= ~(RXNEIE); // Disable UART1 interrupt enable before cleaning status register and data
    USART1_CR3 &= ~(EIE); // Disable UART1 interrupt enable before cleaning status register and data
    // Clear any status register
    (void)USART1_SR;
    (void)USART1_DR;

    //USART1_CR1 |= (RXNEIE); // Enable UART1 RX not empty (data to be read) interrupt enable after cleaning status register and data
    USART1_CR3 |= (EIE); // Enable UART1 RX not empty (data to be read) interrupt enable after cleaning status register and data

    // Clear Channel 5 global interrupts status registers
    DMA1_IFCR = (CTEIF5|CHTIF5|CTCIF5|CGIF5);

    DMA1_CMAR5 = (uint32_t)data; // Memory address Page 288
    DMA1_CNDTR5 = (uint16_t)data_size; // Memory size

    uart1_control.status_register = 0;
    uart1_control.uart_callback = receive_uart_callback;

    init_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout, timeout);
    // We need to use __atomic here because process_uart1_time_event is always running
    __atomic_store_n(&uart1_control.start_monitore, true, __ATOMIC_RELEASE); // Starts monitoring before enable DMA 5 and UART1

    DMA1_CCR5 |= DMA1_CCR5_EN; // Enable DMA1 Channel 5 receive
    USART1_CR1 |= RE;  // Ensure Receive is enable
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
  }

  return UART_OK;
#endif
}

enum uart1_status_t uart1_transmit(
  uint8_t *data, size_t data_size,
  uart_callback_func transmit_uart_callback,
  uint32_t timeout
)
{

  if ((data == NULL) || (data_size == 0))
    return UART_OK;

  if (uart1_is_busy())
    return UART_BUSY;

  TIMEOUT_MS timeout_ms;
  if (!sys_try_lock(&uart1_control.locked, &timeout_ms, 1, NULL)) // 1 milliseconds to wait
    return UART_LOCKED;

   // We need to use __atomic here because process_uart1_time_event is always running
   __atomic_store_n(&uart1_control.start_monitore, false, __ATOMIC_RELEASE);

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
//  USART1_CR1 &= ~(TCIE);
  // Prepare MAX485 driver transmit mode
  MASTER_RS485_DRIVER_TRANSMIT_MODE
#endif

  DMA1_CCR4 &= ~(DMA1_CCR4_EN); // Disable DMA1_Channel4 (transmit)
  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5 (receive)
  USART1_CR1 &= ~(RE);  // Ensure Receive is disable (receive)

  //USART1_CR1 &= ~(RXNEIE); // Disable UART1 interrupt enable before cleaning status register and data
  USART1_CR3 &= ~(EIE); //// Disable UART1 interrupt enable before cleaning status register and data
  // Clear any status register
  (void)USART1_SR;
  (void)USART1_DR;

  // Clear Channel 4 global interrupts status registers
  DMA1_IFCR = (CTEIF4|CHTIF4|CTCIF4|CGIF4);

  uart1_control.status_register = 0;
  uart1_control.uart_callback = transmit_uart_callback;

  DMA1_CMAR4 = (uint32_t)data; // Memory address Page 288

  DMA1_CNDTR4 = (uint16_t)data_size;

  //uart1_control.timeout = timeout;
  init_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout, timeout);
  // We need to use __atomic here because process_uart1_time_event is always running
  __atomic_store_n(&uart1_control.start_monitore, true, __ATOMIC_RELEASE); // Starts monitoring before enable DMA 4

  DMA1_CCR4 |= DMA1_CCR4_EN; // Enable DMA1 Channel 4 transmit

  return UART_OK;
}

/**
 * @brief Periodic UART1 event processor (acts as a soft watchdog).
 *
 * This function must be called periodically (every 1ms).
 * It monitors UART1 DMA transfer and receive states, detects timeouts,
 * ensures DMA1/USART1 are disabled on failure, and calls the registered
 * callback with the final status (success, error, timeout).
 *
 * Concurrency rules:
 *  - uart1_control.locked prevents concurrent TX/RX operations.
 *  - ISR may access master_rs485_rtu only when locked = true.
 *  - Timeout always releases lock and disables DMA safely.
 */
void process_uart1_time_event()
{
  if (__atomic_load_n(&uart1_control.start_monitore, __ATOMIC_SEQ_CST)) {

    uint32_t status_register = (uint32_t)__atomic_load_n(&uart1_control.status_register, __ATOMIC_SEQ_CST);

    switch (status_register) {
      case UART1_TRANSFER_COMPLETE:
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
         if (USART1_SR & TC)
           goto process_uart1_time_event_finish;
#endif
         if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout))
           goto process_uart1_time_event_timeout_error;
        // If RS485 master: Do nothing. Timer2 ISR will resolve UART1_TRANSFER_COMPLETE event or execute error on timeout
        break;
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
      case UART1_RECEIVE_COMPLETE:
          if (USART1_SR & RXNE)
            goto process_uart1_time_event_finish;

          if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout))
            goto process_uart1_time_event_timeout_error;
        break;
#endif
      default:

        if (status_register) // Unknown error
          goto process_uart1_time_event_finish;

        if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout)) {

process_uart1_time_event_timeout_error:
          status_register = E_UART1_TIMEOUT;

process_uart1_time_event_finish:

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
          TIM2_CR1 &= ~(CEN); // Disable Timer 2
          TIM2_SR &= ~(UIF); // Disable overflow flag
          TIM2_CNT = 0;
#endif
          DMA1_CCR4 &= ~(DMA1_CCR4_EN); // Ensure Disable DMA1_Channel4
          DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Ensure Disable DMA1_Channel5
          USART1_CR1 &= ~(RE);  // Ensure Receive is disable

          //USART1_CR1 &= ~(RXNEIE); // Ensure Disable UART1 interrupt enable before cleaning status register and data
          USART1_CR3 &= ~(EIE); // Ensure Disable UART1 interrupt enable before cleaning status register and data

          // Clear interrupts and status registers
          (void)USART1_SR;
          (void)USART1_DR;

          __atomic_store_n(&uart1_control.status_register, 0, __ATOMIC_RELEASE);
          __atomic_store_n(&uart1_control.start_monitore, false, __ATOMIC_RELEASE); // Stop monitoring
          __atomic_store_n(&uart1_control.locked, false, __ATOMIC_RELEASE); // Same as sys_unlock
#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
          MASTER_RS485_DRIVER_TRANSMIT_MODE
#endif
          uart1_control.uart_callback(status_register);
        }
    }
  }
}


/////////////UART2/RS485 slave implementation

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
volatile struct uart_control_t uart2_control = {0};

#else

#include <rs485.h>

struct uart2_control_t {
  volatile bool start_monitore;               // UART start timeout monitore event
  volatile TIMEOUT_MS timeout;                // UART timeout
  volatile uint32_t status_register;          // UART status register
  uart_callback_func uart_callback;           // Uart transmit/receive callback event
};

volatile struct uart2_control_t uart2_control = {0};

extern uint8_t modbus_slave_buffer[];
extern SOLAR48_RS485_RTU_SLAVE slave_rs485_rtu;
extern int slave_send_resp(uint8_t **, size_t *);
extern void rs485_slave_transmit_callback(int);
extern void rs485_slave_receive_callback(int);

void uart2_receive(uint8_t *, size_t, uart_callback_func);
static void uart2_transmit(uint8_t *, size_t, uart_callback_func);

#define _RS485_SLAVE_START_LISTEN uart2_receive(&modbus_slave_buffer[0], MODBUS_SLAVE_BUFFER_SIZE, rs485_slave_receive_callback);

static const struct slave_rs485_speed_t {
  uint32_t uart2_speed;
  uint16_t tim3_adj_transmit;
  uint16_t tim3_adj_receive;
} SLAVE_RS485_SPEED[] = {
    {UART2_2_4_KBPS, 952, 908},
    {UART2_9_6_KBPS, 476, 454},
    {UART2_19_2_KBPS, 336, 321},
    {UART2_57_6_KBPS, 194, 185},
    {UART2_115_2_KBPS, 137, 131},
    {UART2_230_769_KBPS, 97, 92},
    {UART2_461_538_KBPS, 68, 65},
    {UART2_923_076_KBPS, 47, 46},
    {UART2_2250_KBPS, 31, 29}
};

static uint16_t tim3_adj_transmit;
static uint16_t tim3_adj_receive;

#endif

// DMA1 for UART2 Rx events IRQ
void DMA1_Channel6_IRQHandler()
{
  uint32_t dma1_ch6_sr = DMA1_ISR;

  // Clear Channel 6 global interrupts status registers
  DMA1_IFCR = (CTEIF6|CHTIF6|CTCIF6|CGIF6);

//  DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Disable DMA1_Channel5

  // DMA1 error on receive
  if (dma1_ch6_sr & TEIF6) {
    __atomic_store_n(&uart2_control.status_register, E_UART2_DMA1_CH6_RECEIVE_ERROR, __ATOMIC_RELEASE);
    return;
  }

  // Receive complete
  if (dma1_ch6_sr & TCIF6) {
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
    __atomic_store_n(&uart2_control.status_register, UART2_RECEIVE_COMPLETE, __ATOMIC_RELEASE);
#else
    __atomic_store_n(&uart2_control.status_register, E_RS485_SLAVE_DMA6_BUFFER_OVERFLOW, __ATOMIC_RELEASE);
#endif
  }
}

// DMA1 for UART2 Tx events IRQ
void DMA1_Channel7_IRQHandler()
{
  uint32_t dma1_ch7_sr = DMA1_ISR;

  // Clear Channel 7 global interrupts status registers
  DMA1_IFCR = (CTEIF7|CHTIF7|CTCIF7|CGIF7);

  DMA1_CCR7 &= ~(DMA1_CCR7_EN); // Disable DMA1_Channel7

  // DMA1 error on transfer
  if (dma1_ch7_sr & TEIF7) {
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
    SLAVE_RS485_DRIVER_RECEIVE_MODE // If error on transmit disables immediatelly transmit mode (high impedance bus)
#endif
    __atomic_store_n(&uart2_control.status_register, E_UART2_DMA1_CH7_TRANSMIT_ERROR, __ATOMIC_RELEASE);
    return;
  }

  // Transfer complete
  if (dma1_ch7_sr & TCIF7) {
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2

    TIM3_PSC = tim3_adj_transmit;
    TIM3_ARR = tim3_adj_transmit - 1;

    TIM3_SR &= ~(UIF); // Disable overflow flag
    TIM3_CNT = 0;
    TIM3_CR1 |= CEN; // Enable Timer 3

#endif
    __atomic_store_n(&uart2_control.status_register, UART2_TRANSFER_COMPLETE, __ATOMIC_RELEASE);

  }
}

//Table 196. USART interrupt requests page 816
void USART2_IRQHandler()
{
  // Clear any status register
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  uint32_t uart2_has_error = (USART2_SR & (ORE|NE|FE|PE));
  (void)USART2_DR;
#else
  uint32_t status = USART2_SR;
  (void)USART2_DR;

  uint32_t uart2_has_error = (status & (ORE|NE|FE|PE));
#endif

  if (uart2_has_error) {

    DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Ensure Disable DMA1_Channel6
    USART2_CR1 &= ~(RE);  // Ensure Receive is disable
    __atomic_store_n(&uart2_control.status_register, E_UART2_RECEIVE_ERROR_BASE | uart2_has_error, __ATOMIC_RELEASE);

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
    return;
#endif
  }

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
// READ Continuous communication using DMA Page 812

  if (status & IDLE) {

    DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Ensure Disable DMA1_Channel6
    USART2_CR1 &= ~(RE);  // Ensure Receive is disable

    TIM3_PSC = tim3_adj_receive;
    TIM3_ARR = tim3_adj_receive - 1;

    TIM3_SR &= ~(UIF); // Disable overflow flag
    TIM3_CNT = 0;
    TIM3_CR1 |= CEN; // Enable Timer 3

    __atomic_store_n(&uart2_control.status_register, UART2_RECEIVE_COMPLETE, __ATOMIC_RELEASE);

    return;
  }

  __atomic_store_n(&uart2_control.status_register, E_RS485_SLAVE_UART2_IRQ_ERROR, __ATOMIC_RELEASE);

#endif
}

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
void TIM3_IRQHandler()
{
  if (TIM3_SR & UIF) {

    TIM3_SR &= ~(UIF);

    int status = __atomic_load_n(&uart2_control.status_register, __ATOMIC_SEQ_CST);

    if (status == UART2_RECEIVE_COMPLETE) {
      uint8_t *data;
      size_t data_size;
      int result = slave_send_resp(&data, &data_size);

      if (data)
        uart2_transmit(data, data_size, rs485_slave_transmit_callback);
      else if (result == 0)
        _RS485_SLAVE_START_LISTEN
      else if (__atomic_load_n(&slave_rs485_rtu.enable_listening_debug, __ATOMIC_SEQ_CST))
        __atomic_store_n(&uart2_control.status_register, result, __ATOMIC_RELEASE);
      else
        _RS485_SLAVE_START_LISTEN

      return;
    }

    if (status == UART2_TRANSFER_COMPLETE) {
      _RS485_SLAVE_START_LISTEN
      return;
    }

    SLAVE_RS485_DRIVER_RECEIVE_MODE;
    DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Disable DMA1_Channel6
    DMA1_CCR7 &= ~(DMA1_CCR7_EN); // Disable DMA1_Channel7
    USART2_CR1 &= ~(RE);  // Ensure Receive is disable
    USART2_CR1 &= ~(TE); // Ensure Disable Transmi
    __atomic_store_n(&uart2_control.status_register, E_RS485_SLAVE_TIM3_IRQ_ERROR, __ATOMIC_RELEASE);
  }
}
#endif

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
void init_uart2(enum uart2_speed_e speed, enum uart2_mode_e mode)
#else
int init_slave_rs485(uint8_t slave_address, enum rs485_slave_speed_e speed, enum rs485_slave_mode_e mode, uint32_t timeout_on_error, bool enable_listening_debug)
#endif
{
  // 7.3.7 - APB2 peripheral clock enable register (RCC_APB2ENR) Page 112
  RCC_APB2ENR |= (IOPAEN|AFIOEN);   // Enables GPIOA. Page 113
  RCC_APB1ENR |= USART2EN; // Enables USART2. Page 115

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2

  RCC_APB1ENR |= TIM3EN; // Enable Timer 3 page 115

  // ---- Control pin (PA1) for MAX485 ----
  // PA1 -> Output pin for MAX485 DE/RE_B. Default 0
  //9.2.1 Port configuration register low (GPIOx_CRL) (x=A..G) Page 171
  GPIOA_CRL &= ~(GPIOA_MODE1_VAL(0b11) | GPIOA_CNF1_VAL(0b11));
  GPIOA_CRL |= GPIOA_MODE1_VAL(0b11) | GPIOA_CNF1_VAL(0b00); // Output mode, max speed 50 MHz and GPIO as output mode

  SLAVE_RS485_DRIVER_RECEIVE_MODE // Enable RS485 Slave receive
#endif

  // 7.3.6 AHB peripheral clock enable register (RCC_AHBENR) Page 111
  RCC_AHBENR |= DMA1EN;

  // ---- Configure GPIOA TX ----
  // PA2 = TX (AF push-pull), PA3 = RX (input floating)
  //9.2.1 Port configuration register low (GPIOx_CRL) (x=A..G) Page 171
  GPIOA_CRL &= ~(GPIOA_MODE2_VAL(0b11) | GPIOA_CNF2_VAL(0b11));
  GPIOA_CRL |= GPIOA_MODE2_VAL(0b11) | GPIOA_CNF2_VAL(0b10); // Output mode, max speed 50 MHz and Alternate function output Push-pull

  // ---- Configure GPIOA RX ----
  // PA2 = TX (AF push-pull), PA3 = RX (input floating)
  //9.2.1 Port configuration register low (GPIOx_CRL) (x=A..G) Page 171
  GPIOA_CRL &= ~(GPIOA_MODE3_VAL(0b11) | GPIOA_CNF3_VAL(0b11));
  GPIOA_CRL |= GPIOA_MODE3_VAL(0b00) | GPIOA_CNF3_VAL(0b01); // Floating input mode (reset state)

//  USART2_CR1 |= UE;      // Enable UART2

  // Set UART 2 Speed
  //See page 798: 27.3.4 Fractional baud rate generation
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  USART2_BRR = SLAVE_RS485_SPEED[speed].uart2_speed;
#else
  USART2_BRR = (uint32_t)speed;
#endif

  USART2_CR3 = (
                  DMAT | // DMA enable transmitter
                  DMAR | // DMA enable receiver
                  EIE    // Error interrupt enable
               );

  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART2_CR1 = (
                 ((uint32_t)mode)|
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
                 TE // Transmit enable
#else
                 //RE| // Receive enable in RS 485 Slave mode
                 //RXNEIE|
                 IDLEIE // Idle interrupt enabled
#endif
               );

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  TIM3_CR1 = OPM; // One Pulse Mode activated for TIMER3 page 404
  TIM3_CNT = 0; // Reset counter page 418

  tim3_adj_transmit = SLAVE_RS485_SPEED[speed].tim3_adj_transmit; // Load transmit;
  tim3_adj_receive = SLAVE_RS485_SPEED[speed].tim3_adj_receive; // Load receive

  TIM3_PSC = tim3_adj_receive;
  TIM3_ARR = tim3_adj_receive - 1;
  TIM3_SR &= ~(UIF); // Reset overflow flag
  TIM3_DIER = UIE; // Timer 3 interrupt enable
#endif

  USART2_CR1 |= UE;      // Enable UART2

  dma1_channel6_init((void *)&USART2_DR);
  dma1_channel7_init((void *)&USART2_DR);

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  __nvic_set_priority(TIM3_IRQn, TIM3_PRIO); // Set timer 3 priority
  __nvic_enable_irq(TIM3_IRQn); // Enable IRQ for TIMER3
#endif

  __nvic_set_priority(USART2_IRQn, UART2_PRIO);
  __nvic_enable_irq(USART2_IRQn);

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  int err = 0;

  if (slave_address < 1 || slave_address > 247) {
    err = E_RS485_INVALID_SLAVE_ADDRESS_SET_TO_ONE;
    slave_address = 1;
  }

  slave_rs485_rtu.slave_address = slave_address;
  slave_rs485_rtu.timeout_ms = timeout_on_error;
  slave_rs485_rtu.enable_listening_debug = enable_listening_debug;

  return err;
#endif
}

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
inline bool uart2_is_busy()
{
  return (((DMA1_CCR7 & DMA1_CCR7_EN) != 0) || ((USART2_CR1 & RE) != 0));
}

enum uart2_status_t uart2_receive(
  uint8_t *data, size_t data_size,
  uart_callback_func receive_uart_callback,
  uint32_t timeout
)
#else
void uart2_receive(
  uint8_t *data, size_t data_size,
  uart_callback_func receive_uart_callback
)
#endif
{

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  if ((data != NULL) && (data_size > 0)) {

    if (uart2_is_busy())
      return UART2_BUSY;

    TIMEOUT_MS timeout_ms;
    if (!sys_try_lock(&uart2_control.locked, &timeout_ms, 1, NULL)) // 1 milliseconds to wait
      return UART2_LOCKED;
#else
    SLAVE_RS485_DRIVER_RECEIVE_MODE
#endif
    // We need to use __atomic here because process_uart2_time_event is always running
    __atomic_store_n(&uart2_control.start_monitore, false, __ATOMIC_RELEASE);

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
    //27.6.4 Control register 1 (USART_CR1) Page: 821
    USART2_CR1 &= ~(TE); // Disable Transmit disable
#endif

    USART2_CR1 &= ~(RE);           // Ensure Receive is disable
    DMA1_CCR6 &= ~(DMA1_CCR6_EN);  // Disable DMA1_Channel6
    DMA1_CCR7 &= ~(DMA1_CCR7_EN);  // Disable DMA1_Channel7

    //USART2_CR1 &= ~(RXNEIE); // Disable UART2 interrupt enable before cleaning status register and data
    USART2_CR3 &= ~(EIE); // Disable UART2 interrupt enable before cleaning status register and data
    // Clear any status register
    (void)USART2_SR;
    (void)USART2_DR;

    USART2_CR3 |= (EIE); // Enable UART2 RX interrupt enable after cleaning status register and data
    //USART2_CR1 |= (RXNEIE); // Enable UART2 RX not empty (data to be read) interrupt enable after cleaning status register and data

    // Clear Channel 6 global interrupts status registers
    DMA1_IFCR = (CTEIF6|CHTIF6|CTCIF6|CGIF6);

    DMA1_CMAR6 = (uint32_t)data; // Memory address Page 288
    DMA1_CNDTR6 = (uint16_t)data_size; // Memory size

    uart2_control.status_register = 0;
    uart2_control.uart_callback = receive_uart_callback;

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
    init_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout, timeout);
#endif
    // We need to use __atomic here because process_uart2_time_event is always running
    __atomic_store_n(&uart2_control.start_monitore, true, __ATOMIC_RELEASE); // Starts monitoring before enable DMA 6 and UART2

    DMA1_CCR6 |= DMA1_CCR6_EN; // Enable DMA1 Channel 6 receive
    USART2_CR1 |= RE;  // Ensure Receive is enable
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  }

  return UART2_OK;
#endif
}

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
enum uart2_status_t uart2_transmit(
  uint8_t *data, size_t data_size,
  uart_callback_func transmit_uart_callback,
  uint32_t timeout
)
#else

inline void rs485_slave_start_listen()
{
  _RS485_SLAVE_START_LISTEN
}

static void uart2_transmit(
  uint8_t *data, size_t data_size,
  uart_callback_func transmit_uart_callback
)
#endif
{

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  if ((data == NULL) || (data_size == 0))
    return UART2_OK;

  if (uart2_is_busy())
    return UART2_BUSY;

  TIMEOUT_MS timeout_ms;
  if (!sys_try_lock(&uart2_control.locked, &timeout_ms, 1, NULL)) // 1 milliseconds to wait
    return UART2_LOCKED;
#else
   SLAVE_RS485_DRIVER_TRANSMIT_MODE
#endif
   // We need to use __atomic here because process_uart2_time_event is always running
   __atomic_store_n(&uart2_control.start_monitore, false, __ATOMIC_RELEASE);

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART2_CR1 &= ~(TE); // Disable Transmit disable
#endif

  DMA1_CCR7 &= ~(DMA1_CCR7_EN); // Disable DMA1_Channel7 (transmit)
  DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Disable DMA1_Channel6 (receive)
  USART2_CR1 &= ~(RE);  // Ensure Receive is disable (receive)

  //USART2_CR1 &= ~(RXNEIE); // Disable UART2 interrupt enable before cleaning status register and data
  USART2_CR3 &= ~(EIE); // Disable UART2 interrupt enable before cleaning status register and data
  // Clear any status register
  (void)USART2_SR;
  (void)USART2_DR;

  // Clear Channel 7 global interrupts status registers
  DMA1_IFCR = (CTEIF7|CHTIF7|CTCIF7|CGIF7);

  uart2_control.status_register = 0;
  uart2_control.uart_callback = transmit_uart_callback;

  DMA1_CMAR7 = (uint32_t)data; // Memory address Page 288
  DMA1_CNDTR7 = (uint16_t)data_size;

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  //uart2_control.timeout = timeout;
  init_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout, timeout);
#endif

  // We need to use __atomic here because process_uart2_time_event is always running
  __atomic_store_n(&uart2_control.start_monitore, true, __ATOMIC_RELEASE); // Starts monitoring before enable DMA 7


  DMA1_CCR7 |= DMA1_CCR7_EN; // Enable DMA1 Channel 7 transmit

#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART2_CR1 |= TE; // Enable Transmit
#endif

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  return UART2_OK;
#endif
}

/**
 * @brief Periodic UART2 event processor (acts as a soft watchdog).
 *
 * This function must be called periodically (every 1ms).
 * It monitors UART2 DMA transfer and receive states, detects timeouts,
 * ensures DMA1/USART2 are disabled on failure, and calls the registered
 * callback with the final status (success, error, timeout).
 *
 * Concurrency rules:
 *  - uart2_control.locked prevents concurrent TX/RX operations.
 *  - ISR may access slave_rs485_rtu only when locked = true.
 *  - Timeout always releases lock and disables DMA safely.
 */
void process_uart2_time_event()
{
  if (__atomic_load_n(&uart2_control.start_monitore, __ATOMIC_SEQ_CST)) {

    uint32_t status_register = (uint32_t)__atomic_load_n(&uart2_control.status_register, __ATOMIC_SEQ_CST);

    switch (status_register) {
      // 0: idle mode (monitoring, transmit complete or receive complete it will do nothing for RS485 Slave mode)
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
      case 0:
#endif
      case UART2_TRANSFER_COMPLETE:
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
         if (USART2_SR & TC)
           goto process_uart2_time_event_finish;

         if (is_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout))
           goto process_uart2_time_event_timeout_error;
        // If RS485 slave: Do nothing. Timer3 ISR will resolve UART2_TRANSFER_COMPLETE event
        break;
#endif
      case UART2_RECEIVE_COMPLETE:
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
          if (USART2_SR & RXNE)
            goto process_uart2_time_event_finish;

          if (is_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout))
            goto process_uart2_time_event_timeout_error;
#endif
        // If RS485 slave: Do nothing. Timer3 ISR will resolve UART2_RECEIVE_COMPLETE event
        break;
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
      case RS485_SLAVE_WAIT_RESTART:
        if (is_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout)) {
          __atomic_store_n(&uart2_control.status_register, 0, __ATOMIC_RELEASE);
          _RS485_SLAVE_START_LISTEN
        }

        break;
#endif
      default:
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
        if (status_register) // Unknown error
          goto process_uart2_time_event_finish;

        if (is_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout)) {

process_uart2_time_event_timeout_error:
          status_register = E_UART2_TIMEOUT;

process_uart2_time_event_finish:
#else
          SLAVE_RS485_DRIVER_RECEIVE_MODE
          TIM3_CR1 &= ~(CEN); // Disable Timer 3
          TIM3_SR &= ~(UIF); // Disable overflow flag
          TIM3_CNT = 0;
#endif

          DMA1_CCR7 &= ~(DMA1_CCR7_EN); // Ensure Disable DMA1_Channel7
          DMA1_CCR6 &= ~(DMA1_CCR6_EN); // Ensure Disable DMA1_Channel6
          USART2_CR1 &= ~(RE);  // Ensure Receive is disable
#ifdef IMPLEMENT_RS485_SLAVE_OVER_UART2
          //27.6.4 Control register 1 (USART_CR1) Page: 821
          USART2_CR1 &= ~(TE); // Ensure Disable Transmit
#endif

          //USART2_CR1 &= ~(RXNEIE); // Ensure Disable UART2 interrupt enable before cleaning status register and data
          USART2_CR3 &= ~(EIE); // Ensure Disable UART2 interrupt enable before cleaning status register and data

          // Clear interrupts and status registers
          (void)USART2_SR;
          (void)USART2_DR;

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
          __atomic_store_n(&uart2_control.status_register, 0, __ATOMIC_RELEASE);
          __atomic_store_n(&uart2_control.start_monitore, false, __ATOMIC_RELEASE); // Stop monitoring
          __atomic_store_n(&uart2_control.locked, false, __ATOMIC_RELEASE); // Same as sys_unlock
#else
          init_timeout_ms((TIMEOUT_MS *)&uart2_control.timeout, slave_rs485_rtu.timeout_ms);
          __atomic_store_n(&uart2_control.status_register, RS485_SLAVE_WAIT_RESTART, __ATOMIC_RELEASE);
#endif

          uart2_control.uart_callback(status_register);
#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
        }
#endif
    }
  }
}

