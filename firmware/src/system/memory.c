#include <memory.h>

extern uint32_t _estack;
extern uint32_t _end;
extern uint32_t __ram_length;
extern uint32_t _etext;
extern uint32_t __flash_origin;
extern uint32_t __flash_length;

uint32_t size = (uint32_t)&__ram_length;

#define U8_PATTERN (uint8_t)(0xAA)
#define U32_MEM_PATTERN (uint32_t)(U8_PATTERN<<24)|(U8_PATTERN<<16)|(U8_PATTERN<<8)|(U8_PATTERN<<0)

void fill_stack_with_pattern(void)
{
  uintptr_t end = (uintptr_t)&_estack;
  uintptr_t aligned_end = end & ~(0x03);

  uint32_t *p32 = (uint32_t *)&_end;

  while ((uintptr_t)p32 < aligned_end)
    *(p32++) = U32_MEM_PATTERN;

  uint8_t *p8 = (uint8_t *)p32;
  while ((uintptr_t)p8 < end)
   *(p8++) = U8_PATTERN;
}

static uint32_t get_stack_peak_usage(void)
{
  uintptr_t end = (uintptr_t)&_estack;
  uint32_t *p32 = (uint32_t*)&_end;
  uintptr_t aligned_end = end & ~(0x03);
  uint32_t left = 0;

#define CHECK_U32_PATTERN(n) \
  if ((tmp & (U8_PATTERN<<n)) == (U8_PATTERN<<n)) \
    left++;

  while ((uintptr_t)p32 < aligned_end)
    if (*(p32++) != (U32_MEM_PATTERN)) {

      if ((uintptr_t)p32 == aligned_end)
        break;

      uint32_t tmp = *(--p32);

      CHECK_U32_PATTERN(0)
      CHECK_U32_PATTERN(8)
      CHECK_U32_PATTERN(16)
      CHECK_U32_PATTERN(24)

      p32++;
    }

#undef CHECK_U32_PATTERN

  uint8_t *p8 = (uint8_t *)p32;
  while (((uintptr_t)p8 < end) && (*(p8) == U8_PATTERN))
    p8++;

  return (uint32_t)&_estack - (uint32_t)p8 + left;
}

void get_ram_detailed(DETAILED_RAM *dr)
{
  uint32_t current_stack;
  uint32_t total_available = (uint32_t)&_estack - (uint32_t)&_end;

  dr->size            = size;
  dr->total_available = total_available;

  __asm volatile ("mrs %0, msp" : "=r" (current_stack));

  dr->stack_used      = (uint32_t)&_estack - current_stack;
  dr->percent_used = 100 - (100*(total_available - dr->stack_used)) / size;
  dr->stack_peak_used = get_stack_peak_usage();
}

void get_flash_detailed(DETAILED_FLASH *df)
{
  df->size         = (uint32_t)&__flash_length;
  df->used         = (uint32_t)&_etext - (uint32_t)&__flash_origin;
  df->percent_used = (100*df->used)/df->size;
}

#undef U32_MEM_PATTERN
#undef U8_PATTERN
#undef GET_MSP

