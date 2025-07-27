import re
import sys
import matplotlib.pyplot as plt

def parse_map_file(map_path):
    memory = {
        'text': 0,
        'rodata': 0,
        'data': 0,
        'bss': 0,
        'comment': 0,
        'isr_vector': 0,
        'ARM.exidx': 0,
    }

    section_pattern = re.compile(r'^\s*(\.?[\w\.]+)\s+0x[\da-fA-F]+\s+0x([\da-fA-F]+)')
    
    with open(map_path, 'r', errors='ignore') as f:
        for line in f:
            match = section_pattern.match(line)
            if match:
                section = match.group(1).replace('.', '')
                size = int(match.group(2), 16)
                if section in memory:
                    memory[section] += size

    return {k: v for k, v in memory.items() if v > 0}

def generate_pie_chart(sections, output_image=None):
    labels = list(sections.keys())
    sizes = list(sections.values())

    plt.figure(figsize=(8, 8))
    plt.pie(sizes, labels=labels, autopct='%1.1f%%', startangle=140)
    plt.title("Memory section distribution")
    plt.axis('equal')
    plt.tight_layout()

    if output_image:
        plt.savefig(output_image, dpi=150)
        print(f"[+] Saved graphic: {output_image}")
    else:
        plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Use: python3 analyze_and_plot_map.py file.map [out.png]")
        sys.exit(1)

    map_file = sys.argv[1]
    output_image = sys.argv[2] if len(sys.argv) > 2 else None

    memory_usage = parse_map_file(map_file)
    
    if not memory_usage:
        print("[!] No section found.")
    else:
        print("== Section memory use ==")
        for k, v in memory_usage.items():
            print(f"{k:<15}: {v:>8} bytes")
        generate_pie_chart(memory_usage, output_image)

