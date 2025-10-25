#include <registers.h>
#include <sys_interrupts.h>
#include <dma.h>
#include <solar48_config.h>
#include <time.h>
#include <system.h>
#include <hal_uart.h>

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

// DMA1 for UART1 Rx events IRQ
void DMA1_Channel5_IRQHandler()
{
  uint32_t dma1_ch5_sr = DMA1_ISR;

  // Clear Channel 4 global interrupts status registers
  DMA1_IFCR = (CTEIF5|CHTIF5|CTCIF5|CGIF5);

  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Disable DMA1_Channel5

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

    return;
  }

  // DMA1 error on receive
  if (dma1_ch5_sr & TEIF5)
    __atomic_store_n(&uart1_control.status_register, E_UART1_DMA1_CH5_RECEIVE_ERROR, __ATOMIC_RELEASE);
}

//Table 196. USART interrupt requests page 816
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

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
void TIM2_IRQHandler()
{
  if (TIM2_SR & UIF) {

    TIM2_SR &= ~(UIF);

    uart1_receive(modbus_master_buffer, (size_t)master_rs485_rtu.first_pass_len, _master_receive, master_rs485_rtu.timeout_ms);
  }
}
#endif

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
void init_uart1(enum uart1_speed_e speed)
#else
void init_master_rs485(enum rs485_master_speed_e speed)
#endif
{

  // 7.3.7 - APB2 peripheral clock enable register (RCC_APB2ENR) Page 112
  RCC_APB2ENR |= IOPAEN;   // Enables GPIOA. Page 113
  RCC_APB2ENR |= USART1EN; // Enables USART1. Page 113

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1

  RCC_APB1ENR |= TIM2EN; // Enable Timer 2 page 115

  // ---- Control pin (PA8) for MAX485 ----
  // PA8 -> Output pin for MAX485 DE/RE_B. Default 1
  //9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G) Page 172
  GPIOA_CRH &= ~(GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b11));
  GPIOA_CRH |= GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b00); // Output mode, max speed 50 MHz and GPIO as output mode
//  GPIOA_CRH |= GPIOA_MODE8_VAL(0b11) | GPIOA_CNF8_VAL(0b10); // Output alternat function for PA8 and max speed 50Mhz

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
                 PCE|M|  //Parit control enable and stop bit + 9 bit
                 /*RE|*/
                 PEIE| // Parity error interrupt enabled
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
  MASTER_RS485_DRIVER_TRANSMIT_MODE // Enable RS485 Master transmit pin
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
enum uart_status_t
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

    USART1_CR1 &= ~(RXNEIE); // Disable UART1 interrupt enable before cleaning status register and data
    // Clear any status register
    (void)USART1_SR;
    (void)USART1_DR;

    USART1_CR1 |= (RXNEIE); // Enable UART1 RX not empty (data to be read) interrupt enable after cleaning status register and data

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

enum uart_status_t uart1_transmit(
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

  USART1_CR1 &= ~(RXNEIE); // Disable UART1 interrupt enable before cleaning status register and data
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
 * This function must be called periodically (e.g. every 1ms).
 * It monitors UART1 DMA transfer and receive states, detects timeouts,
 * ensures DMA/USART are disabled on failure, and calls the registered
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
         else if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout))
           goto process_uart1_time_event_timeout_error;
#endif
        // If RS485 master: Do nothing. Timer2 ISR will resolve UART1_TRANSFER_COMPLETE event
        break;
      case UART1_RECEIVE_COMPLETE:
          if (USART1_SR & RXNE)
            goto process_uart1_time_event_finish;
          else if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout))
            goto process_uart1_time_event_timeout_error;
        break;
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

          USART1_CR1 &= ~(RXNEIE); // Ensure Disable UART1 interrupt enable before cleaning status register and data

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

