#ifndef MEMORY_H
  #define MEMORY_H

#include <stdint.h>

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

#endif

