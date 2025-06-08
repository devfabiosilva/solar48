/* STM32F103C8T6 - 64 KB Flash, 20 KB RAM */

/* Entry point */
ENTRY(_solar48_entrypoint)

/* Hardware definition for GCC compiler */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM); /* Top of stack */

__ram_origin = ORIGIN(RAM); /* Position of begining of SRAM address */
__ram_length = LENGTH(RAM); /* Length of defined HARDWARE: STM32F103C8T6 - 64 KB Flash, 20 KB RAM */
__flash_origin = ORIGIN(FLASH); /* Position of begining of FLASH address */
__flash_length = LENGTH(FLASH); /* Length of defined HARDWARE: STM32F103C8T6 - 64 KB Flash, 20 KB RAM */

SECTIONS
{
  /* Hardware interrupt code vector page 98  */
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .ARM.exidx :
  {
    *(.ARM.exidx*)
    *(.gnu.linkonce.armexidx.*)
  } > FLASH

  /* Firmware instructions compiled from Assembly and C code */
  .text :
  {
    *(.text .text.* .gnu.linkonce.t.*)
    *(.rodata .rodata.* .gnu.linkonce.r.*)   /* Consts */
    _etext = .;
  } > FLASH

  /* Initialize and copy flash data to SRAM */
  _sidata = _etext;

  /* Start of SRAM data */
  .data : AT(_sidata)
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM

  /* Unitialized variable block */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  /* Optional Heap */
  .heap (COPY):
  {
    _sheap = .;
    . = . + 1K;
    _eheap = .;
  } > RAM

  /* EOP. End of SRAM */

  end = .;
  _end = end;
}

