#include <registers.h>
#include <hal_uart.h>
#include <sys_interrupts.h>
#include <dma.h>
#include <solar48_config.h>
#include <time.h>
#include <system.h>

// #define UART1_TRANSMIT_USE_BLOCK

struct uart_control_t {
  volatile bool start_monitore;               // UART start timeout monitore event
  volatile bool locked;                       // UART locked
  volatile TIMEOUT_MS timeout;                // UART timeout
  volatile uint32_t status_register;          // UART status register
#ifdef UART1_TRANSMIT_USE_BLOCK
  volatile uint8_t *next_data_ptr;            // Data pointer
  volatile int32_t block;                     // Data is divided by n x UART1_TX_RX_BUF
  volatile int32_t left;                      // Data remaining left = data size % UART1_TX_RX_BUF
#endif
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

  // Transfer complete
  if (dma1_ch4_sr & TCIF4) {
#ifdef UART1_TRANSMIT_USE_BLOCK

    //we don't need __atomic here because it is used only here 
    if (uart1_control.block > 0) {

      //we don't need __atomic here because it is used only here
      DMA1_CMAR4 = (uint32_t)uart1_control.next_data_ptr; // Memory address Page 288
      DMA1_CNDTR4 = (uint16_t)UART1_TX_RX_BUF; // Memory size Page 287 (64 KB max)
      --uart1_control.block;
      uart1_control.next_data_ptr += UART1_TX_RX_BUF;
      DMA1_CCR4 |= (DMA1_CCR4_EN); // Enable DMA1_Channel4

    } else if (uart1_control.left) {

      DMA1_CMAR4 = (uint32_t)uart1_control.next_data_ptr; // Memory address Page 288
      DMA1_CNDTR4 = (uint16_t)uart1_control.left; // Memory size Page 287 (64 KB max)
      uart1_control.left = 0;
      DMA1_CCR4 |= (DMA1_CCR4_EN); // Enable DMA1_Channel4

    } else
#endif
      __atomic_store_n(&uart1_control.status_register, UART1_TRANSFER_COMPLETE, __ATOMIC_RELEASE);

    return;
  }

  // DMA1 error on transfer
  if (dma1_ch4_sr & TEIF4)
    __atomic_store_n(&uart1_control.status_register, E_UART1_DMA1_CH4_TRANSMIT_ERROR, __ATOMIC_RELEASE);
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

//27 Universal synchronous asynchronous receiver transmitter (USART) Page 785

void init_uart1()
{

  // 7.3.7 - APB2 peripheral clock enable register (RCC_APB2ENR) Page 112
  RCC_APB2ENR |= IOPAEN;   // Enables GPIOA. Page 113
  RCC_APB2ENR |= USART1EN; // Enables USART1. Page 113

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
  USART1_BRR = UART1_DEFAULT_SPEED;

  USART1_CR3 = (
                  DMAT | // DMA enable transmitter
                  DMAR | // DMA enable receiver
                  EIE    // Error interrupt enable
               );

  //27.6.4 Control register 1 (USART_CR1) Page: 821
  USART1_CR1 = (
                 PCE|M|  //Parit control enable and stop bit + 9 bit
                 /*RE|*/
                 PEIE| // Parity error interrupt enabled
                 TE // Transmit enable
               );

  USART1_CR1 |= UE;      // Enable UART1


  dma1_channel4_init((void *)&USART1_DR);
  dma1_channel5_init((void *)&USART1_DR);

  __nvic_set_priority(USART1_IRQn, UART1_PRIO);
  __nvic_enable_irq(USART1_IRQn);

}

inline bool uart1_is_busy()
{
  return (((DMA1_CCR4 & DMA1_CCR4_EN) != 0) || ((USART1_CR1 & RE) != 0));
}

enum uart_status_t uart1_receive(
  uint8_t *data, size_t data_size,
  uart_callback_func receive_uart_callback,
  uint32_t timeout
)
{
  if ((data != NULL) && (data_size > 0)) {

    if (uart1_is_busy())
      return UART_BUSY;

    TIMEOUT_MS timeout_ms;
    if (!sys_try_lock(&uart1_control.locked, &timeout_ms, 1, NULL)) // 1 milliseconds to wait
      return UART_LOCKED;

//    uart1_control.locked = true;
//    uart1_control.start_monitore = false;
    // We need to use __atomic here because process_uart1_time_event is always running
    __atomic_store_n(&uart1_control.start_monitore, false, __ATOMIC_RELEASE);

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
  }
 
  return UART_OK;
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

//    uart1_control.locked = true;
//    uart1_control.start_monitore = false;
   // We need to use __atomic here because process_uart1_time_event is always running
   __atomic_store_n(&uart1_control.start_monitore, false, __ATOMIC_RELEASE);

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

#ifdef UART1_TRANSMIT_USE_BLOCK
  uart1_control.next_data_ptr = data;
  int32_t block = (int32_t)data_size / UART1_TX_RX_BUF;
  int32_t left = (int32_t)data_size % UART1_TX_RX_BUF;

  if (block > 0) {
    DMA1_CNDTR4 = (uint16_t)UART1_TX_RX_BUF; // Memory size Page 287 (64 KB max)
    uart1_control.next_data_ptr += UART1_TX_RX_BUF;
    --block;
  } else
    DMA1_CNDTR4 = (uint16_t)left; // Memory size Page 287 (64 KB max)

  uart1_control.block = block;
  uart1_control.left = left;
#else
  DMA1_CNDTR4 = (uint16_t)data_size;
#endif

  //uart1_control.timeout = timeout;
  init_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout, timeout);
    // We need to use __atomic here because process_uart1_time_event is always running
    __atomic_store_n(&uart1_control.start_monitore, true, __ATOMIC_RELEASE); // Starts monitoring before enable DMA 4

  DMA1_CCR4 |= DMA1_CCR4_EN; // Enable DMA1 Channel 4 transmit

  return UART_OK;
}

void process_uart1_time_event()
{
  if (__atomic_load_n(&uart1_control.start_monitore, __ATOMIC_SEQ_CST)) {

    uint32_t status_register = (uint32_t)__atomic_load_n(&uart1_control.status_register, __ATOMIC_SEQ_CST);

    switch (status_register) {
      case UART1_TRANSFER_COMPLETE:
          if (USART1_SR & TC)
            goto process_uart1_time_event_finish;
          else if (is_timeout_ms((TIMEOUT_MS *)&uart1_control.timeout))
            goto process_uart1_time_event_timeout_error;
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
          uart1_control.uart_callback(status_register);
        }
    }
  }
}

