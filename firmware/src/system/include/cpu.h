#ifndef CPU_H
 #define CPU_H
#include <stdint.h>

typedef struct cpu_t {
  uint16_t flash_size;
  uint16_t cpu_uid0;
  uint16_t cpu_uid1;
  uint32_t cpu_uid2;
  uint32_t cpu_uid3;
} CPU;

void cpu_info(CPU *);

#endif

