ARM_PREFIX = arm-none-eabi-
CC = $(ARM_PREFIX)gcc
OBJCOPY = $(ARM_PREFIX)objcopy
HAS_RTOS=-DRTOS_SOLAR48
#HAS_RTOS= 
CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g3 -Wall -nostdlib -ffreestanding -DSOLAR48_DEBUG $(HAS_RTOS)
CFLAGS_RELEASE = -mcpu=cortex-m3 -mthumb -O2 -Wall -nostdlib -ffreestanding -Wl,--strip-debug $(HAS_RTOS)
CUR_DIR = $(shell pwd)
FIRMWARE_FOLDER=firmware
ASSEMBLY_FOLDER=$(FIRMWARE_FOLDER)/assembly
SRC_FOLDER=$(FIRMWARE_FOLDER)/src
PREFIX_MCU_TYPE=_stm32f103x6
STARTUP_FILE=startup$(PREFIX_MCU_TYPE)
LINKER_FILE=linker$(PREFIX_MCU_TYPE)
#LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections
LDFLAGS = -T $(ASSEMBLY_FOLDER)/$(LINKER_FILE).ld -Wl,--gc-sections -lc -mfloat-abi=soft -lgcc -lnosys --specs=nano.specs -u malloc -u free -u memcmp -u strlen -u vsnprintf -u strncmp -u memset -u strncpy -u strtol -u _printf_float -u __aeabi_uldivmod -u __aeabi_i2f -u __aeabi_fmul -u __aeabi_fdiv -u snprintf
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

PERIPH_FOLDER=$(SYSTEM_FOLDER)/peripheral

PERIPH_SD1306_FOLDER = $(PERIPH_FOLDER)/ssd1306
PERIPH_SD1306_SRC = $(wildcard $(PERIPH_SD1306_FOLDER)/*.c)
PERIPH_SD1306_OBJ = $(patsubst $(PERIPH_SD1306_FOLDER)/%.c,%.o,$(PERIPH_SD1306_SRC))
PERIPH_SD1306_OBJ_RELEASE = $(patsubst $(PERIPH_SD1306_FOLDER)/%.c,%_release.o,$(PERIPH_SD1306_SRC))

ifeq ($(HAS_RTOS),)
 MIDDLEWARE_FREE_RTOS_SRC=
 MIDDLEWARE_FREE_RTOS_OBJ=
 MIDDLEWARE_FREE_RTOS_OBJ_RELEASE=

 MIDDLEWARE_INSTANCES_SRC =
 MIDDLEWARE_INSTANCES_OBJ =
 MIDDLEWARE_INSTANCES_OBJ_RELEASE =
else
 MIDDLEWARE_FOLDER_FREE_RTOS=$(SRC_FOLDER)/middlewares/FREE_RTOS
 MIDDLEWARE_FREE_RTOS_SRC = $(wildcard $(MIDDLEWARE_FOLDER_FREE_RTOS)/*.c)
 MIDDLEWARE_FREE_RTOS_OBJ = $(patsubst $(MIDDLEWARE_FOLDER_FREE_RTOS)/%.c,%.o,$(MIDDLEWARE_FREE_RTOS_SRC))
 MIDDLEWARE_FREE_RTOS_OBJ_RELEASE = $(patsubst $(MIDDLEWARE_FOLDER_FREE_RTOS)/%.c,%_release.o,$(MIDDLEWARE_FREE_RTOS_SRC))

 MIDDLEWARE_FOLDER_INSTANCES=$(SRC_FOLDER)/middlewares/INSTANCES
 MIDDLEWARE_INSTANCES_SRC = $(wildcard $(MIDDLEWARE_FOLDER_INSTANCES)/*.c)
 MIDDLEWARE_INSTANCES_OBJ = $(patsubst $(MIDDLEWARE_FOLDER_INSTANCES)/%.c,%.o,$(MIDDLEWARE_INSTANCES_SRC))
 MIDDLEWARE_INSTANCES_OBJ_RELEASE = $(patsubst $(MIDDLEWARE_FOLDER_INSTANCES)/%.c,%_release.o,$(MIDDLEWARE_INSTANCES_SRC))
endif

OUT = solar48
AS_CODE=text.txt
AS_CODE_RELEASE=text_release.txt

all: $(OUT).bin

%.o: $(SYSTEM_FOLDER)/%.c
	@echo "DEBUG: Compiling System modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(PERIPH_SD1306_FOLDER)/%.c
	@echo "DEBUG: Compiling peripheral ssd1306 driver ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/USB_DEVICE/%.c
	@echo "DEBUG: Compiling USB middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/CONSOLE/%.c
	@echo "DEBUG: Compiling CONSOLE middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/FREE_RTOS/%.c
	@echo "DEBUG: Compiling FREE RTOS middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%.o: $(MIDDLEWARE_FOLDER)/INSTANCES/%.c
	@echo "DEBUG: Compiling INSTANCES middleware modules ..."
	$(CC) $(CFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

$(OUT).elf: $(SRC_FOLDER)/main.c $(SYS_OBJ) $(PERIPH_SD1306_OBJ) $(MIDDLEWARE_CONSOLE_OBJ) $(MIDDLEWARE_USB_OBJ) $(MIDDLEWARE_FREE_RTOS_OBJ) $(MIDDLEWARE_INSTANCES_OBJ) $(ASSEMBLY_FOLDER)/$(STARTUP_FILE).S
	$(CC) $(CFLAGS) -Wl,-Map=solar48.map $(LDFLAGS) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -o $@ $^

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

openocd: $(OUT).elf
	@echo "Running OpenOCD ... $(CUR_DIR)"
	@openocd -f $(CUR_DIR)/debugger/stlink.cfg -f $(CUR_DIR)/debugger/stm32f1x.cfg

gdb: $(OUT).elf
	@gdb-multiarch $(OUT).elf

%_release.o: $(SYSTEM_FOLDER)/%.c
	@echo "RELEASE: Compiling System modules ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(PERIPH_SD1306_FOLDER)/%.c
	@echo "RELEASE: Compiling peripheral ssd1306 driver ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/USB_DEVICE/%.c
	@echo "RELEASE: Compiling USB middlewares module ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/CONSOLE/%.c
	@echo "RELEASE: Compiling CONSOLE middlewares module ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/FREE_RTOS/%.c
	@echo "RELEASE: Compiling FREE RTOS middleware modules ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

%_release.o: $(MIDDLEWARE_FOLDER)/INSTANCES/%.c
	@echo "RELEASE: Compiling INSTANCES middleware modules ..."
	$(CC) $(CFLAGS_RELEASE) -I$(SYSTEM_FOLDER_INC) -I$(MIDDLEWARE_FOLDER_INC) -c -o $@ $^

$(OUT)_release.elf: $(SRC_FOLDER)/main.c $(SYS_OBJ_RELEASE) $(PERIPH_SD1306_OBJ_RELEASE) $(MIDDLEWARE_CONSOLE_OBJ_RELEASE) $(MIDDLEWARE_USB_OBJ_RELEASE) $(MIDDLEWARE_FREE_RTOS_OBJ_RELEASE) $(MIDDLEWARE_INSTANCES_OBJ_RELEASE) $(ASSEMBLY_FOLDER)/$(STARTUP_FILE).S
	$(CC) $(CFLAGS_RELEASE) -Wl,-Map=solar48_release.map $(LDFLAGS) -I$(MIDDLEWARE_FOLDER_INC) -I$(SYSTEM_FOLDER_INC) -o $@ $^

$(OUT)_release.bin: $(OUT)_release.elf
	$(OBJCOPY) -O binary $< $@

nm_release: $(OUT)_release.elf
	$(ARM_PREFIX)nm $(OUT)_release.elf

asm_release: $(OUT)_release.elf
	$(ARM_PREFIX)objdump -D $(OUT)_release.elf > $(AS_CODE_RELEASE)

flash_release: $(OUT)_release.bin
	st-flash write $(OUT)_release.bin 0x8000000

