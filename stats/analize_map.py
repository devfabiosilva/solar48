import sys
import re
from collections import defaultdict

SECTION_RAM = [".bss", ".data", ".heap", ".stack"]
SECTION_FLASH = [".text", ".rodata", ".isr_vector", ".init", ".fini"]

def parse_map_file(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    in_linker_map = False
    memory_usage = defaultdict(int)

    for line in lines:
        if 'Linker script and memory map' in line:
            in_linker_map = True
            continue

        if not in_linker_map:
            continue

        match = re.match(r'^\s+\.(\S+)\s+0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)', line)
        if match:
            section = match.group(1)
            size = int(match.group(2), 16)
            if any(section.startswith(s) for s in SECTION_RAM):
                memory_usage['RAM'] += size
            elif any(section.startswith(s) for s in SECTION_FLASH):
                memory_usage['FLASH'] += size
            memory_usage[f'section: {section}'] += size

    return memory_usage

def print_memory_report(mem):
    print("\n== Memory Report ==")
    print(f"Total RAM usage:   {mem['RAM']} bytes")
    print(f"Total FLASH usage: {mem['FLASH']} bytes\n")

    print("== Per Section Usage ==")
    for key, val in sorted(mem.items(), key=lambda x: x[1], reverse=True):
        if key.startswith('section:'):
            print(f"{key[9:]:<20}: {val:>7} bytes")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 analyze_map.py <file.map>")
        sys.exit(1)

    mem = parse_map_file(sys.argv[1])
    print_memory_report(mem)

