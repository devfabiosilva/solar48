#include <stdint.h>
#include <stddef.h>
#include <registers.h>
#include <sys_interrupts.h>

// 13 Direct memory access controller (DMA) Page 274

// --- Table 78. Summary of DMA1 requests for each channel Pag 282 ---
// DMA1 for UART1 Tx events IRQ
void DMA1_Channel4_IRQHandler()
{

}

// DMA1 for UART1 Rx events IRQ
void DMA1_Channel5_IRQHandler()
{

}
// ---------------------------------------------------------------------

// Note: Used for USART1_TX only
void dma1_channel4_init(void *memory_address, size_t memory_size, void *peripheral_address)
{
  DMA1_CCR4 &= ~(DMA1_CCR4_EN); // Before configure. Disable DMA1 Page 286
  DMA1_CPAR4 = (uint32_t)peripheral_address; // Peripheral address Page 288
  DMA1_CMAR4 = (uint32_t)memory_address; // Memory address Page 288
  DMA1_CNDTR4 = (uint16_t)memory_size; // Memory size Page 287 (64 KB max)

  DMA1_CCR4 |= (
                 DMA1_PL4_SEL(0b10) | // Priority HIGH
                 DMA1_DIR4  |         // Data read direction: Read from memory
                 DMA1_MINC4 |         // Memory increment mode
                 DMA1_TCIE4 |         // Transfer complete interrupt enable
                 DMA1_TEIE4           // DMA error interrupt enable
               );

  DMA1_CCR4 |= DMA1_CCR4_EN; // Enable DMA1 Channel 4 for process UART1 TX

  __nvic_set_priority(DMA1_Channel4_IRQn, DMA1_CH4_PRIO); // Set DMA1 Channel 4 interrupt Priority
  __nvic_enable_irq(DMA1_Channel4_IRQn); // Enable DMA1 Channel 4 interrupt

}

// Note: Used for USART1_RX only
void dma1_channel5_init(void *memory_address, size_t memory_size, void *peripheral_address)
{
  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Before configure. Disable DMA1 Page 286
  DMA1_CPAR5 = (uint32_t)peripheral_address; // Peripheral address Page 288
  DMA1_CMAR5 = (uint32_t)memory_address; // Memory address Page 288
  DMA1_CNDTR5 = (uint16_t)memory_size; // Memory size Page 287 (64 KB max)

  DMA1_CCR5 |= (
                 DMA1_PL5_SEL(0b10) | // Priority HIGH
                 DMA1_MINC5 |         // Memory increment mode
                 DMA1_TCIE5 |         // Transfer complete interrupt enable
                 DMA1_TEIE5           // DMA error interrupt enable
               );

  DMA1_CCR5 |= DMA1_CCR5_EN; // Enable DMA1 Channel 4 for process UART1 TX

  __nvic_set_priority(DMA1_Channel5_IRQn, DMA1_CH5_PRIO); // Set DMA1 Channel 5 interrupt Priority
  __nvic_enable_irq(DMA1_Channel5_IRQn); // Enable DMA1 Channel 5 interrupt
}

