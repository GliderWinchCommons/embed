# Makefile - Cortex-M3, STM32F103
# 01-20-2012

# Name of the .c file with 'main' as well as output file name
NAME		= linear_batt_chgr

# The path for the commands to find the compiler, linker, et al.
PREFIX = $(ARM_PREFIX)/bin/arm-none-eabi-
CC		= $(PREFIX)gcc
LD		= $(PREFIX)ld
OBJCOPY		= $(PREFIX)objcopy
OBJDUMP		= $(PREFIX)objdump


# Current directory
SRCDIR=.
# Library directory path
LIBLIB		= $(SRCDIR)/../../sw_stm32/trunk
LIBDIR		= $(LIBLIB)/lib
LIBSUP		= $(LIBLIB)/lib/libsupportstm32
LIBDEV		= $(LIBLIB)/devices

# Library directory path
LIBDEVICESSTM32	 = $(LIBDEV)
LIBMISCSTM32	 = $(LIBDIR)/libmiscstm32
LIBOPENSTM32	 = $(LIBDIR)/libopenstm32
LIBUSARTSTM32	 = $(LIBDIR)/libusartstm32
LIBUSUPPORTSTM32 = $(LIBDIR)/libsupportstm32

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra  -I$(LIBDEV) -I$(LIBDIR) -I$(LIBUSUPPORTSTM32) -Iinclude -fno-common \
		  -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc


AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
LDSCRIPT	= $(NAME).ld

# Path to lc lm lgcov lgcc and other good libraries
LCC = $(ARM_PREFIX)/arm-none-eabi/lib/armv7-m
LOTHER = $(ARM_PREFIX_GCCLIB)

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS         = -L$(LOTHER) -lgcc -lgcov  -L$(LCC) -lc -lm -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles 
LDFLAGS += -L$(LIBMISCSTM32) 
LDFLAGS += -L$(LIBUSARTSTM32) 
LDFLAGS += -L$(LIBDEVICESSTM32) 
LDFLAGS += -L$(LIBUSUPPORTSTM32)

# List of files to be compiled
OBJS		=  vector.o $(NAME).o


# Paths for openocd and automatic flashing
OPENOCD_BASE	= /usr/share
OPENOCD		= /usr/bin/openocd
OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/openocd/scripts
OPENOCD_FLASHER	= $(OPENOCD_SCRIPTS)/interface/rlink.cfg
OPENOCD_BOARD	= $(OPENOCD_SCRIPTS)/board/olimex_stm32_h103.cfg

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
endif

all: images



images: $(NAME)
#	@printf "  OBJCOPY $(NAME).bin\n"
#	$(Q)$(OBJCOPY) -Obinary $(NAME) $(NAME).bin
	@printf "  OBJCOPY $(NAME).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(NAME) $(NAME).hex
	@printf "  OBJCOPY $(NAME).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(NAME) $(NAME).srec
	@printf "  OBJDUMP $(NAME).list\n"
	$(Q)$(OBJDUMP) -S $(NAME) > $(NAME).list

.c.o:
#	$(CC)	$(CFLAGS) -S -o $@ $<
	$(CC)	$(CFLAGS) -c -o $@ $<
.s.o:
	$(AS)	$(AFLAGS) -a=$*.lst	-o $@ $<


$(NAME): $(OBJS) $(LDSCRIPT)
	@printf "  LD      $(subst $(shell pwd)/,,$(@)) $(LFLAGS)\n"
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)  -lm -lgcc -lc -lgcc -lmiscstm32 -lusartstm32 -ldevicesstm32 -lsupportstm32
clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o $(LIBOPENSTM32)/*.o
	@printf "  CLEAN   $(NAME):"
	$(Q)rm -f $(NAME)
	@printf " .bin"
#	$(Q)rm -f $(NAME).bin
#	@printf " .hex"
	$(Q)rm -f $(NAME).hex
	@printf " .srec"
	$(Q)rm -f $(NAME).srec
	@printf " .list"
	$(Q)rm -f $(NAME).list
	@printf " .map\n "
	$(Q)rm -f $(NAME).map


flash: images
	@printf "  FLASH   $(NAME).srec\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	$(Q)$(OPENOCD) -s $(OPENOCD_SCRIPTS) \
		       -f $(OPENOCD_FLASHER) \
		       -f $(OPENOCD_BOARD) \
		       -c "init" -c "reset halt" \
		       -c "flash write_image erase $(NAME).srec" \
		       -c "reset" \
		       -c "shutdown" $(NULL)

.PHONY: images clean

