ARM_PREFIX = arm-none-eabi-
CC = $(ARM_PREFIX)gcc
OBJCOPY = $(ARM_PREFIX)objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g -Wall -nostdlib -ffreestanding -Wl,-Map=solar48.map -DSOLAR48_DEBUG
CFLAGS_RELEASE = -mcpu=cortex-m3 -mthumb -O2 -Wall -nostdlib -ffreestanding -Wl,--strip-debug -Wl,-Map=solar48_release.map
PWD=$(pwd)
FIRMWARE_FOLDER=firmware
ASSEMBLY_FOLDER=$(FIRMWARE_FOLDER)/assembly
SRC_FOLDER=$(FIRMWARE_FOLDER)/src
PREFIX_MCU_TYPE=_stm32f103x6
STARTUP_FILE=startup$(PREFIX_MCU_TYPE)
LINKER_FILE=linker$(PREFIX_MCU_TYPE)
#LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections
LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections -lc -mfloat-abi=soft -lgcc -lnosys --specs=nano.specs -u malloc -u free -u memcmp -u strlen -u vsnprintf -u strncmp -u memset -u strncpy -u strtol -u _printf_float -u __aeabi_uldivmod
SYSTEM_FOLDER=$(SRC_FOLDER)/system
SYSTEM_FOLDER_INC=$(SYSTEM_FOLDER)/include
MIDDLEWARE_FOLDER=$(SRC_FOLDER)/middlewares
MIDDLEWARE_FOLDER_INC=$(MIDDLEWARE_FOLDER)/include
MIDDLEWARE_FOLDER_USB=$(SRC_FOLDER)/middlewares/USB_DEVICE
MIDDLEWARE_FOLDER_CONSOLE=$(SRC_FOLDER)/middlewares/CONSOLE


SYS_SRC = $(wildcard $(SYSTEM_FOLDER)/*.c)
SYS_OBJ = $(patsubst $(SYSTEM_FOLDER)/%.c,%.o,$(SYS_SRC))
SYS_OBJ_RELEASE = $(patsubst $(SYSTEM_FOLDER)/%.c,%_release.o,$(SYS_SRC))

MIDDLEWARE_USB_SRC = $(wildcard $(MIDDLEWARE_FOLDER_USB)/*.c) 
MIDDLEWARE_USB_OBJ = $(patsubst $(MIDDLEWARE_FOLDER_USB)/%.c,%.o,$(MIDDLEWARE_USB_SRC))
MIDDLEWARE_USB_OBJ_RELEASE = $(patsubst $(MIDDLEWARE_FOLDER_USB)/%.c,%_release.o,$(MIDDLEWARE_USB_SRC))

MIDDLEWARE_CONSOLE_SRC = $(wildcard $(MIDDLEWARE_FOLDER_CONSOLE)/*.c) 
MIDDLEWARE_CONSOLE_OBJ = $(patsubst $(MIDDLEWARE_FOLDER_CONSOLE)/%.c,%.o,$(MIDDLEWARE_CONSOLE_SRC))
MIDDLEWARE_CONSOLE_OBJ_RELEASE = $(patsubst $(MIDDLEWARE_FOLDER_CONSOLE)/%.c,%_release.o,$(MIDDLEWARE_CONSOLE_SRC))

OUT = solar48
AS_CODE=text.txt
AS_CODE_RELEASE=text_release.txt

all: $(OUT).bin

%.o: $(SYSTEM_FOLDER)/%.c
	@echo "DEBUG: Compiling System modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/USB_DEVICE/%.c
	@echo "DEBUG: Compiling USB middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/CONSOLE/%.c
	@echo "DEBUG: Compiling CONSOLE middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

$(OUT).elf: $(SRC_FOLDER)/main.c $(SYS_OBJ) $(MIDDLEWARE_CONSOLE_OBJ) $(MIDDLEWARE_USB_OBJ) $(ASSEMBLY_FOLDER)/$(STARTUP_FILE).S
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -o $@ $^

$(OUT).bin: $(OUT).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f *.map *.elf *.bin *.o $(AS_CODE) $(AS_CODE_RELEASE)

flash: $(OUT).bin
	st-flash write $(OUT).bin 0x8000000

asm: $(OUT).elf
	$(ARM_PREFIX)objdump -D $(OUT).elf > $(AS_CODE)

nm: $(OUT).elf
	$(ARM_PREFIX)nm $(OUT).elf


%_release.o: $(SYSTEM_FOLDER)/%.c
	@echo "RELEASE: Compiling System modules ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/USB_DEVICE/%.c
	@echo "RELEASE: Compiling USB middlewares module ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/CONSOLE/%.c
	@echo "RELEASE: Compiling CONSOLE middlewares module ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

$(OUT)_release.elf: $(SRC_FOLDER)/main.c $(SYS_OBJ_RELEASE) $(MIDDLEWARE_CONSOLE_OBJ_RELEASE) $(MIDDLEWARE_USB_OBJ_RELEASE) $(ASSEMBLY_FOLDER)/$(STARTUP_FILE).S
	$(CC) $(CFLAGS_RELEASE) $(LDFLAGS) -I$(MIDDLEWARE_FOLDER_INC) -I$(SYSTEM_FOLDER_INC) -o $@ $^

$(OUT)_release.bin: $(OUT)_release.elf
	$(OBJCOPY) -O binary $< $@

nm_release: $(OUT)_release.elf
	$(ARM_PREFIX)nm $(OUT)_release.elf

asm_release: $(OUT)_release.elf
	$(ARM_PREFIX)objdump -D $(OUT)_release.elf > $(AS_CODE_RELEASE)

flash_release: $(OUT)_release.bin
	st-flash write $(OUT)_release.bin 0x8000000

