set pagination off
target remote :3333

set $ram_start = (unsigned int)&_sdata
set $ram_end   = (unsigned int)&_estack
set $program_name = "solar48"

define mem_stats
  printf "\n==== SRAM USAGE STATS ====\n"

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

define snapshot

   set $file_program_name_bin = "solar48.bin"
   set $file_program_name_hex = "solar48.hex"

   echo "=== Pause CPU ===\n"
   monitor halt

   dump binary memory "sram_solar48.bin" $sram_start $sram_end

   shell arm-none-eabi-objcopy -I binary -O ihex $file_program_name_bin $file_program_name_hex

   echo "=== Resume CPU ===\n"
   monitor resume

   echo "=== Saved Snapshot: ===\n"
   shell ls -lh $filename_bin $filename_hex
end

#define autosnap
#    set $count = 0
#    while ($count < $arg0)
#        snapshot
#        shell sleep $arg1
#        set $count = $count + 1
#    end
#end

document snapshot
    Single SRAM in binary and HEX.
    Use: snapshot
end

#document autosnap
#    Multiples snapshot at every seconds.
#    Use: autosnap <quantity> <interval_in_seconds>
#end

echo "Default configuration loaded. See mem_stats to find memory\n"
echo "SRAM: See help snapshot for details\n"

