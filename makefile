# make a f c ca i p t r-<variable>

################################################################################

TARGET := nemesis-app
BOOTLOADER := nemesis-boot

DEBUG := 0
OPTIM := O1

ASRCS := $(sort $(shell find . -name '*.s' -printf '%f '))
AOBJS := $(addprefix objs/,$(ASRCS:.s=.o))

CSRCS := $(sort $(shell find src -name '*.c' -printf '%f '))
CDEPS := $(patsubst %.c,deps/%.d,$(CSRCS))
COBJS := $(addprefix objs/,$(CSRCS:.c=.o))

ASFLAGS := -Wa,--warn -Wa,--fatal-warnings
CPPFLAGS := -I inc -I inc/pt -I inc/uip -I /usr/arm-none-eabi/include
CFLAGS := -march=armv6-m -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -mlittle-endian -ffreestanding -fsigned-char -fdata-sections -ffunction-sections -Wall -Werror -Wno-unused-but-set-variable -$(OPTIM)
LDFLAGS := -nostdlib -nostartfiles -nodefaultlibs -Llibs -L/usr/arm-none-eabi/lib/thumb/v6-m/nofp -L/usr/lib/gcc/arm-none-eabi/8.2.0/thumb/v6-m/nofp -T $(TARGET).ld -Wl,-Map=$(TARGET).map -Wl,--cref -Wl,--gc-sections -Wl,--print-memory-usage -Wl,--stats -Wl,--print-output-form
LDLIBS := -lm -lgcc -lc_nano -lnosys

ifeq ($(DEBUG),1)
   CFLAGS += -g
endif

vpath %.h inc:inc/pt:inc/uip
vpath %.s src
vpath %.c src:src/uip

$(shell if [ ! -d deps ]; then mkdir -p deps; fi)
$(shell if [ ! -d objs ]; then mkdir -p objs; fi)

################################################################################

$(TARGET).elf : $(AOBJS) $(COBJS)
	$(info Linking $(TARGET))
	arm-none-eabi-gcc $^ $(LDFLAGS) $(LDLIBS) -o $@
	arm-none-eabi-objdump -d -S -z -w $@ > $(TARGET).lst
	arm-none-eabi-objcopy -O ihex $@ $(TARGET).hex
	arm-none-eabi-objcopy -O binary $@ $(TARGET).bin
	printf "78563412%.8x%.8x" $$(stat -c "%s" $(TARGET).bin) $$(sum $(TARGET).bin | cut -d' ' -f1) | sed -E 's/(.{8})(..)(..)(..)(..)(..)(..)(..)(..)/\1\5\4\3\2\9\8\7\6/' | xxd -p -r | cat - $(TARGET).bin > $(TARGET).fw
	find . -name '*.c' | xargs -n1 wc -l | sort -n -r | sed --silent '1,5p'
	arm-none-eabi-nm -S --size-sort -r $(TARGET).elf | sed --silent '1,5p'
	arm-none-eabi-size --format=sysv --common -d $(TARGET).elf

$(firstword $(subst -, ,$(TARGET))).bin : $(BOOTLOADER).nib $(TARGET).bin
	cat $^ > $@

objs/%.o : %.s
	arm-none-eabi-gcc $< -c $(CFLAGS) $(ASFLAGS) -o $@

objs/%.o : %.c deps/%.d
	arm-none-eabi-gcc $< -c $(CPPFLAGS) $(CFLAGS) -o $@
	arm-none-eabi-gcc $< -E $(CPPFLAGS) -MM -MP -MT $@ -MF deps/$*.d && touch $@

deps/%.d : ;

.PRECIOUS : deps/%.d

################################################################################

.PHONY : a f c ca i p t r-%

a : $(TARGET).elf

f : $(firstword $(subst -, ,$(TARGET))).bin

c :
	rm -rf *.o *.elf *.bin *.hex *.fw *.map *.lst *.png cscope* tags deps objs

ca : c
	rm -f *.nib

i : a
	~/bin/lpc21isp -donotstart -verify -bin $(firstword $(subst -, ,$(TARGET))).bin /dev/ttyUSB0 115200 12000

p :
	picocom --baud 38400 --databits 8 --stopbits 1 --parity n --flow n --send-cmd 'sx -vv' --receive-cmd 'rx -vv' --logfile 'logs/picocom.log' --echo --quiet /dev/ttyUSB0

t :
	ctags -R --extra=+f *
	find . -name '*.[csh]' > cscope.files
	cscope -q -R -b -i cscope.files

r-% :
	@echo $* = $($*)

################################################################################

-include $(CDEPS)
