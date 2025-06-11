#include <registers.h>
#include <cpu.h>

void cpu_info(CPU *cpuinf)
{
  cpuinf->flash_size = FLASH_SIZE_REGISTER;
  cpuinf->cpu_uid0   = CHIP_UID_BASE_0;
  cpuinf->cpu_uid1   = CHIP_UID_BASE_1;
  cpuinf->cpu_uid2   = CHIP_UID_BASE_2;
  cpuinf->cpu_uid3   = CHIP_UID_BASE_3;
}

