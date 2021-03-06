#------------------------------------------------------------------
# Makefile for compiling a hex for bootloader
#------------------------------------------------------------------
# Change these defs for the target device

MCU_TARGET    = atmega128
# Target device to be used (32K or larger)
BOOT_ADR      = 0x1E000
# Boot loader start address [byte] NOT [word] as in http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega1284p
F_CPU         = 11059200
# CPU clock frequency [Hz] NOT critical: it just should be higher than the actual Hz
#------------------------------------------------------------------
CSRC        =
# C source to compile and link
ASRC        =
# Assembly source to compile and link
TARGET      =
# target name

OPTIMIZE    = -Os -mcall-prologues -ffunction-sections -fdata-sections
DEFS        = -DF_CPU=$(F_CPU)
LIBS        =
DEBUG       = dwarf-2

ASFLAGS     = -Wa,-adhlns=$(<:.S=.lst),-gstabs $(DEFS)
ALL_ASFLAGS = -mmcu=$(MCU_TARGET) -I. -x assembler-with-cpp $(ASFLAGS)
CFLAGS      = -g$(DEBUG) -Wall $(OPTIMIZE) $(ADDED_CFLAGS) -mmcu=$(MCU_TARGET) -std=c99 $(DEFS)
LDFLAGS     = -Wl,-Map,$(TARGET).map -Wl,--gc-sections -Wl,--section-start,.text=$(BOOT_ADR)
OBJ         = $(CSRC:.c=.o) $(ASRC:.S=.o)

CC          = avr-gcc
OBJCOPY     = avr-objcopy
OBJDUMP     = avr-objdump
SIZE        = avr-size


all:	clean $(TARGET).elf lst text bin size

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(TARGET).elf *.eps *.bak *.a *.bin
	rm -rf pff/src/*.o uart/*.o
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
	rm -rf $(TARGET).hex

size: $(TARGET).elf
	$(SIZE) -C --mcu=$(MCU_TARGET) $(TARGET).elf

lst:  $(TARGET).lst
%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.o : %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

text: $(TARGET).hex
%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .fuse -O ihex $< $@
# --- make bin just to check size :)
bin: $(TARGET).bin
%.bin: %.hex
	$(OBJCOPY) -I ihex -O binary $< $@

print-%  : ; @echo $* = $($*)  #test any var with make print-XXX
