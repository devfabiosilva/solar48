#include <stdint.h>
#include <stddef.h>
#include <registers.h>
#include <sys_interrupts.h>

// ---------------------------------------------------------------------
// Note: Used for USART1_TX only
void dma1_channel4_init(void *peripheral_address)
{
  DMA1_CCR4 &= ~(DMA1_CCR4_EN); // Before configure. Disable DMA1 Page 286
  DMA1_CPAR4 = (uint32_t)peripheral_address; // Peripheral address Page 288

  DMA1_CCR4 |= (
                 DMA1_PL4_SEL(0b10) | // Priority HIGH
                 DMA1_DIR4  |         // Data read direction: Read from memory
                 DMA1_MINC4 |         // Memory increment mode
                 DMA1_TCIE4 |         // Transfer complete interrupt enable
                 DMA1_TEIE4           // DMA error interrupt enable
               );

  __nvic_set_priority(DMA1_Channel4_IRQn, DMA1_CH4_PRIO); // Set DMA1 Channel 4 interrupt Priority
  __nvic_enable_irq(DMA1_Channel4_IRQn); // Enable DMA1 Channel 4 interrupt

}

// Note: Used for USART1_RX only
void dma1_channel5_init(void *peripheral_address)
{
  DMA1_CCR5 &= ~(DMA1_CCR5_EN); // Before configure. Disable DMA1 Page 286
  DMA1_CPAR5 = (uint32_t)peripheral_address; // Peripheral address Page 288

  DMA1_CCR5 |= (
                 DMA1_PL5_SEL(0b10) | // Priority HIGH
                 DMA1_MINC5 |         // Memory increment mode
                 DMA1_TCIE5 |         // Transfer complete interrupt enable
                 DMA1_TEIE5           // DMA error interrupt enable
               );


  __nvic_set_priority(DMA1_Channel5_IRQn, DMA1_CH5_PRIO); // Set DMA1 Channel 5 interrupt Priority
  __nvic_enable_irq(DMA1_Channel5_IRQn); // Enable DMA1 Channel 5 interrupt
}

