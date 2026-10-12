# SPDX-FileCopyrightText: 2024 Mete Balci
#
# SPDX-License-Identifier: GPL-3.0-or-later

# set debug to 0 or 1 (override on the command line, e.g. make debug=0)
# adjust optimization flag accordingly below
debug ?= 1
# set fpu to soft, softfp or hard (override on the command line, e.g. make fpu=hard)
# soft:   software fpu, soft abi
# softfp: hardware fpu, soft abi
# hard:   harwdare fpu, hard abi
fpu ?= soft

# specify an aarch32 bare-metal eabi toolchain
CC = arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

COMMON_SRC = $(wildcard ../../common/*.c)
COMMON_OBJ = $(COMMON_SRC:.c=.o)

GDB     = gdb

# modify these to add/remove different code/object files
C_OBJECTS = $(COMMON_OBJ) core0.o core1.o 
S_OBJECTS = minimum_arm_image_def_block.o
ELF = blink_led_two_cores.elf
BIN = $(ELF:.elf=.bin)
UF2 = $(BIN:.bin=.uf2)

# sets DEBUGFLAGS based on debug above
ifeq ($(debug), 1)
	DEBUGFLAGS = -g3 -O0
else
	# change optimization options to whatever suits you
	DEBUGFLAGS = -O2
endif

# sets FLOATFLAGS based on fpu above
ifeq ($(fpu), softfp)
	FLOATFLAGS = -mfloat-abi=softfp -mfpu=fpv5-sp-d16
else ifeq ($(fpu), hard)
	FLOATFLAGS = -mfloat-abi=hard -mfpu=fpv5-sp-d16
else
	FLOATFLAGS = -mfloat-abi=soft
endif

# cpu target and instruction set
CFLAGS = -mcpu=cortex-m33 -mthumb -std=gnu11
# floating point model
CFLAGS += $(FLOATFLAGS)
# includes
CFLAGS += -I../../../include

# use newlib nano

# put functions and data into individual sections
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall
CFLAGS += $(DEBUGFLAGS)

ASFLAGS = -mcpu=cortex-m33 -mthumb
ASFLAGS += $(FLOATFLAGS)

# enable c preprocessor in assembly source files
ASFLAGS += -x assembler-with-cpp
ASFLAGS += $(DEBUGFLAGS)

LDFLAGS = -mcpu=cortex-m33 -mthumb
LDFLAGS += $(FLOATFLAGS)
# use the linker script
LDFLAGS += -T"linker.ld"

# remove empty sections only if not for debug
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -static

LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

# shared openocd invocation for flash, openocd-server, reset
OPENOCD = openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000"

.PHONY: all clean flash debug openocd-server reset size

all: $(UF2)

clean:
	rm -rf $(UF2) $(BIN) $(ELF) *.o ../../common/*.o

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(ASFLAGS) -c -o $@ $<

$(ELF): $(C_OBJECTS) $(S_OBJECTS) Makefile linker.ld
	$(CC) -o $@ $(C_OBJECTS) $(S_OBJECTS) $(LDFLAGS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(UF2): $(BIN)
	python3 ../../../scripts/uf2conv.py $< \
		-b 0x10000000 \
		-f 0xe48bff59 \
		-o $@

flash: $(ELF)
	$(OPENOCD) -c "program $(ELF) verify reset exit"

debug: $(ELF)
	arm-none-eabi-gdb -ex "target remote localhost:3333" -ex "monitor reset init" -ex "break Reset_Handler"  -ex "monitor arm semihosting enable" $(ELF)

openocd-server:
	$(OPENOCD)

reset:
	$(OPENOCD) -c "init; reset; exit;"

size: $(ELF)
	arm-none-eabi-size $(ELF)


dump_6_1: $(ELF)
	$(GDB) -q $(TARGET) \
	-ex "set pagination off" \
	-ex "set confirm off" \
	-ex "set logging file spy_results.txt" \
	-ex "set logging overwrite on" \
	-ex "set logging enabled on" \
	-ex "file blink_led_two_cores.elf" \
	-ex "target extended-remote :3333" \
	-ex "set print elements 0" \
	-ex "p foo_time" \
	-ex "set print elements 0" \
	-ex "p timestamp_core1" \
	-ex "set print elements 0" \
	-ex "p timestamp_core0" \
	-ex "set print elements 0" \
	-ex "p zero" \
	-ex "set logging enabled off" \
	-ex "dump binary memory spy_results.bin foo_time foo_time+10" \
	-ex "quit"


do_analysis: dump_6_1
	. ../../../scripts/env/bin/activate && \
	python3 ../../../scripts/new.py spy_results.txt