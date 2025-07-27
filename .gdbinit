set pagination off
target remote :3333

define mem_stats
  printf "\n==== SRAM USAGE STATS ====\n"

  set $ram_start = (unsigned int)&_sdata
  set $ram_end   = (unsigned int)&_estack
  set $stack_top = (void*)_estack

  set $data_size = (unsigned int)&_edata - (unsigned int)&_sdata
  set $bss_size  = (unsigned int)&_ebss - (unsigned int)&_sbss
  set $used_ram = (unsigned int)$data_size + (unsigned int)$bss_size
  set $total_ram = (unsigned int)$ram_end - (unsigned int)$ram_start
  set $free_ram = (unsigned int)$total_ram - (unsigned int)$used_ram

  printf "  Total RAM       = %d bytes\n", $total_ram
  printf "  .data size      = %d bytes\n", $data_size
  printf "  .bss size       = %d bytes\n", $bss_size
  printf "  Total used RAM  = %d bytes\n", $used_ram
  printf "  Free RAM        = %d bytes\n", $free_ram
end

document mem_stats
  Shows SRAM stats
  Includes .data and .bss.
end

echo "Default configuration loaded. See mem_stats to find memory\n"

