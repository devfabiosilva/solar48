CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g -Wall -nostdlib -ffreestanding
CFLAGS_RELEASE = -mcpu=cortex-m3 -mthumb -O2 -Wall -nostdlib -ffreestanding -Wl,--strip-debug
PWD=$(pwd)
FIRMWARE_FOLDER=firmware
ASSEMBLY_FOLDER=$(FIRMWARE_FOLDER)/assembly
SRC_FOLDER=$(FIRMWARE_FOLDER)/src
#STARTUP_FILE=startup
PREFIX_MCU_TYPE=_stm32f103x6
STARTUP_FILE=startup$(PREFIX_MCU_TYPE)
LINKER_FILE=linker$(PREFIX_MCU_TYPE)
#LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections
LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections -lc -mfloat-abi=soft -lgcc -lnosys --specs=nano.specs -u malloc -u free
#_printf_float
SYSTEM_FOLDER=$(SRC_FOLDER)/system
SYSTEM_FOLDER_INC=$(SYSTEM_FOLDER)/include
SRC = $(SRC_FOLDER)/main.c $(SYSTEM_FOLDER)/system.c $(SYSTEM_FOLDER)/gpios.c $(SYSTEM_FOLDER)/time.c $(ASSEMBLY_FOLDER)/$(STARTUP_FILE).S
OUT = solar48
AS_CODE=text.txt
AS_CODE_RELEASE=text_release.txt

all: $(OUT).bin

$(OUT).elf: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(SYSTEM_FOLDER_INC) -o $@ $^

$(OUT).bin: $(OUT).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f *.elf *.bin *.o $(AS_CODE) $(AS_CODE_RELEASE)

flash: $(OUT).bin
	st-flash write $(OUT).bin 0x8000000

asm: $(OUT).elf
	arm-none-eabi-objdump -D $(OUT).elf > $(AS_CODE)

nm: $(OUT).elf
	arm-none-eabi-nm $(OUT).elf

$(OUT)_release.elf: $(SRC)
	$(CC) $(CFLAGS_RELEASE) $(LDFLAGS) -I$(SYSTEM_FOLDER_INC) -o $@ $^

$(OUT)_release.bin: $(OUT)_release.elf
	$(OBJCOPY) -O binary $< $@

nm_release: $(OUT)_release.elf
	arm-none-eabi-nm $(OUT)_release.elf

asm_release: $(OUT)_release.elf
	arm-none-eabi-objdump -D $(OUT)_release.elf > $(AS_CODE_RELEASE)

flash_release: $(OUT)_release.bin
	st-flash write $(OUT)_release.bin 0x8000000

