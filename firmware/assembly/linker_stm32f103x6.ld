/* STM32F103C8T6 - 64 KB Flash, 20 KB RAM */

ENTRY(_start)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM); /* Top of stack */

SECTIONS
{
  /* Vetor de interrupções e código */
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text :
  {
    *(.text*)
    *(.rodata*)   /* Constantes */
    _etext = .;
  } > FLASH

  /* Copiar valores inicializados da flash para a RAM */
  _sidata = _etext;

  .data : AT(_sidata)
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM

  /* Variáveis não inicializadas */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  /* Heap opcional */
  .heap (COPY):
  {
    _sheap = .;
    . = . + 1K;
    _eheap = .;
  } > RAM

  /* Stack já definido pelo _estack */

  /* Endereço final do programa */
  _end = .;
}

