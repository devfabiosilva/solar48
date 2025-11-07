#include <memory.h>
#include <stdlib.h>
#include <string.h>

extern uint32_t _estack;
extern uint32_t _end;
extern uint32_t __ram_length;
extern uint32_t _etext;
extern uint32_t __flash_origin;
extern uint32_t __flash_length;
extern void app_panic(const char *);

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

#ifdef SOLAR48_DEBUG
extern uint32_t _eheap;

uint32_t get_free_heap_stack_gap(void) {
  return (uint32_t)&_estack - (uint32_t)&_eheap;
}
#endif

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
#ifdef SOLAR48_DEBUG
  dr->free_heap_stack_gap = get_free_heap_stack_gap();
#endif
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

void *solar48_mem(SOLAR48_MEM *mem, size_t mem_size)
{
  if (mem->memory) {
    if (mem_size) {
      if (mem->size >= mem_size)
        return mem->memory;

      void *tmp = realloc(mem->memory, mem_size);

      if (tmp) {
        mem->memory = tmp;
        mem->size = mem_size;

        return tmp;
      }
    } else {
      free(mem->memory);
      mem->memory = NULL;
      mem->size = 0;

      return NULL;
    }

    free(mem->memory);
    app_panic("slr48:rllc");

    return NULL; // Never reaches here
  }

  if (mem_size) {
    if ((mem->memory = malloc(mem->size = mem_size)))
      return mem->memory;
  } else
    return NULL;

  app_panic("slr48:mllc");

  return NULL; // Never reaches here
}

void swap16_array_fast(uint16_t *arr, size_t len)
{

// rev16 instructions swaps 2 half words, so calling this is faster than simple swap bytes using C.
// Example: a = 0x01020304
// mov R0, #0x01020304
// rev16 R0, R0
// RESULT: 0x02010403
// Refs.: https://developer.arm.com/documentation/ddi0602/2025-06/Base-Instructions/REV16--Reverse-bytes-in-16-bit-halfwords-
//        https://developer.arm.com/documentation/dui0379/e/arm-and-thumb-instructions/rev16

  uint32_t *p = (uint32_t *)arr;
  size_t n = len >> 1; // Divide by 2

  while (n > 0) {
    __asm__("rev16 %0, %0": "=r"(*p): "0"(*p));
    p++;
    --n;
  }

  // If even
  if (len & 1) {
    uint16_t y = *((uint16_t *)p);
    *((uint16_t *)p) = (y >> 8) | (y << 8);
    /*
    uint32_t y;
    memcpy((void *)&y, (void *)p, sizeof(uint16_t)); // Copy only 2 bytes (uint16_t)
    __asm__("rev16 %0, %0" : "+r"(y));
    memcpy((void *)p, (void *)&y, sizeof(uint16_t)); // Copy only 2 bytes back inverted (uint16_t)
    */
  }

}

//Read and implement
//https://developer.arm.com/documentation/dui0472/m/Compiler-specific-Features/--attribute----packed---type-attribute
//https://developer.arm.com/documentation/100748/0624/Alignment-support-in-Arm-Compiler-for-Embedded-6/Unaligned-access-support-in-Arm-Compiler-for-Embedded
// IMPORTANT: Call this function if pointer is not aligned
void swap16_array_fast_safe(void *arr, size_t len)
{

  // If aligned always run fast
  if (IS_ALIGNED_32(arr)) {
    swap16_array_fast(arr, len);
    return;
  }

  // If not aligned
  uint32_t 
    two_swap16_at_once,
    *u32_ptr = (uint32_t *)arr;
  int32_t
    u32_len = (int32_t)(len >> 1); // Divide by 2

  while (u32_len > 0) {
    memcpy((void *)&two_swap16_at_once, (void *)u32_ptr, sizeof(two_swap16_at_once));
    __asm__("rev16 %0, %0" : "+r"(two_swap16_at_once));
    memcpy((void *)u32_ptr, (void *)&two_swap16_at_once, sizeof(two_swap16_at_once));
    ++u32_ptr;
    --u32_len;
  }

  if (len & 1) {
    memcpy((void *)&two_swap16_at_once, (void *)u32_ptr, sizeof(uint16_t)); //Only 2 bytes left => sizeof(uint16_t)
    __asm__("rev16 %0, %0" : "+r"(two_swap16_at_once));
    memcpy((void *)u32_ptr, (void *)&two_swap16_at_once, sizeof(uint16_t)); // First 2 bytes only will be copied back and swapped
  }
}

inline void move_uint8_safe(void *dest, uint8_t value)
{
  uint8_t val = value;
  memcpy((void *)dest, (void *)&val, sizeof(val));
}

inline void swap_and_move_uint16_safe(void *dest, uint16_t src)
{
  uint32_t y = (uint32_t)src;
  __asm__("rev16 %0, %0" : "+r"(y));

  memcpy(dest, (void *)&y, sizeof(src)); // Copy 2 bytes (uint16_t)
}

inline uint16_t read_and_swap_uint16_safe(void *src)
{
  uint32_t y;
  memcpy((void *)&y, src, sizeof(uint16_t)); // Read 2 bytes only
  __asm__("rev16 %0, %0" : "+r"(y));

  return (uint16_t)y;
}
// Reads from unaligned uint16_t value a and compares with value swapped b value. Thus return a (from pointer) == b (swapped)
inline bool swap_and_compare_uint16(void *a, uint16_t b)
{
  uint32_t y = (uint32_t)b;
  __asm__("rev16 %0, %0" : "+r"(y));
  return memcmp((void *)a, &y, sizeof(uint16_t)) == 0;
}

// Reads uint8_t from unaligned
inline uint8_t read_uint8(void *a)
{
  return (uint8_t)((uint8_t *)a)[0];
}

