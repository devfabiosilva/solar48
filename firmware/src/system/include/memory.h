#ifndef MEMORY_H
  #define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <system.h>
#include <stdbool.h>

typedef struct detailed_ram_t {
  uint32_t size;
  uint32_t total_available;
  uint32_t stack_used;
  uint32_t stack_peak_used;
  uint32_t percent_used;
#ifdef SOLAR48_DEBUG
  uint32_t free_heap_stack_gap;
#endif
} DETAILED_RAM;

typedef struct detailed_flash_t {
  uint32_t size;
  uint32_t used;
  uint32_t percent_used;
} DETAILED_FLASH;

void fill_stack_with_pattern();
void get_ram_detailed(DETAILED_RAM *dr);
void get_flash_detailed(DETAILED_FLASH *df);

typedef struct solar48_mem_t {
  void *memory;
  size_t size;
} SOLAR48_MEM;

void *solar48_mem(SOLAR48_MEM *, size_t);

void swap16_array_fast(uint16_t *, size_t);
void swap16_array_fast_safe(void *, size_t);
void move_uint8_safe(void *, uint8_t);
void swap_and_move_uint16_safe(void *, uint16_t);
void swap_and_move_two_uint16_at_once_safe(void *, uint32_t);
void swap_and_move_uint16_from_unaligned_to_unaligned_safe(void *, uint16_t *);
uint16_t read_and_swap_uint16_safe(void *src);
bool swap_and_compare_uint16(void *, uint16_t);
uint8_t read_uint8(void *);

#endif

