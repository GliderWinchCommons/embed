# Makefile - Cortex-M3, STM32F103
# 06/06/2013


#
# Note: This version descends into 'libusartstm32' and executes 'make' if
# started by the command 'make', but not when started with the command
# 'make flash'.  
#
# 'make clean' descends into 'libusartstm32'


# Name of the .c file with 'main' as well as output file name
NAME		= ldr

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump
# Current directory
SRCDIR=.

# Library directory path prefix--svn_pod library
LIBPODLIB	= $(SRCDIR)/../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(SRCDIR)/../../../../svn_pod/sw_stm32/trunk/devices
# Library directory path--svn_pod library 
LIBMISCSTM32	= $(LIBPODLIB)/libmiscstm32
LIBOPENSTM32	= $(LIBPODLIB)/libopenstm32
LIBUSARTSTM32	= $(LIBPODLIB)/libusartstm32
LIBSUPPORTSTM32 = $(LIBPODLIB)/libsupportstm32

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(SRCDIR)/../../../sw_f103/trunk/lib

# Library directory path--svn_sensor, f103 library
LIBDEVICEF103SE	= $(LIBSENSORF103)/libdevicestm32
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Library directory path--common includes
LIBCOMMONALL    = $(SRCDIR)/../../../common_all/trunk

# Library directory path--svn_common
LIBSVNCOMMON	= $(SRCDIR)/../../../../svn_common/trunk/f103

# Library directory path--compile and stuff
#LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
#LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# The following is setup in ~/.arm_tools when ~/.profile opens a terminal
LGCC = $(ARM_PREFIX_GCCLIB)
LOTHER = $(ARM_PREFIX_OTHER)

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra\
-I$(LIBDEVICEF103SE) \
-I$(LIBPODLIB) \
-I$(LIBPODDEV) \
-I$(LIBOPENSTM32) \
-I$(LIBMISCSTM32) \
-I$(LIBUSARTSTM32) \
-I$(LIBSUPPORTSTM32)  \
-I$(LIBSENSORMISC) \
-I$(LIBCOMMONALL) \
-Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
LDSCRIPT	= $(NAME).ld
#LDSCRIPT	= $(LIBCOMMONALL)/ldrapp.ld

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS         = -L$(LOTHER) -L$(LGCC) -lgcc -lgcov -lc -lm -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles \
 -L$(LIBDEVICEF103SE) \
 -L$(LIBPODLIB) \
 -L$(LIBPODDEV) \
 -L$(LIBMISCSTM32) \
 -L$(LIBUSARTSTM32) \
 -L$(LIBSUPPORTSTM32) \
 -L$(LIBSENSORMISC) \
 -L$(LIBCOMMONALL) \
 -L$(LIBSVNCOMMON) \

# List of files to be compiled
OBJS	 = vector.o $(NAME).o 
OBJS	+= newlib_support.o 
#OBJS	+= canwinch_ldr.o
OBJS	+= canwinch_ldrproto.o 


# The following two are 'export'ed in .arm_tools via .profile
#OPENOCD_BASE	= /usr/share
#OPENOCD		   = /usr/bin/openocd

OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/openocd/scripts

ifeq ($(JTAG_TYPE),stlink)
 	OPENOCD_FLASHER = $(OPENOCD_SCRIPTS)/interface/stlink-v2.cfg
 	OPENOCD_BOARD	  = $(OPENOCD_SCRIPTS)/board/olimex_stm32_h103_stlink.cfg
else
	OPENOCD_FLASHER = $(OPENOCD_SCRIPTS)/interface/rlink.cfg
	OPENOCD_BOARD	  = $(OPENOCD_SCRIPTS)/board/olimex_stm32_h103.cfg
endif
OPENOCD_TARGET		= $(OPENOCD_SCRIPTS)/target/stm32f1x.cfg

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
endif

all: images

images: $(NAME)
	@printf "  OBJCOPY $(NAME).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(NAME) $(NAME).bin
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
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)  -lm -lgcc -lc -lgcc -lmiscstm32 -lusartstm32 -lsensormisc -ldevicestm32 -lsensormisc -lsupportstm32 -ldevicesstm32 -lc -lm -lgcc -lcommon_allf103 -lc -lgcc
clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o $(LIBOPENSTM32)/*.o
	@printf "  CLEAN   $(NAME):"
	$(Q)rm -f $(NAME)
	@printf " .bin"
	$(Q)rm -f $(NAME).bin
#	@printf " .hex"
#	$(Q)rm -f $(NAME).hex
	@printf " .srec"
	$(Q)rm -f $(NAME).srec
	@printf " .list"
	$(Q)rm -f $(NAME).list
	@printf " .map\n "
	$(Q)rm -f $(NAME).map


flash: images
	@printf "  FLASH   $(NAME).srec\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) \
		       -f $(OPENOCD_FLASHER) \
		       -f $(OPENOCD_BOARD) \
		       -c "init" -c "reset halt" \OPENOCD_TARGET
		       -c "flash write_image erase $(NAME).srec" \
		       -c "reset" \
		       -c "shutdown" $(NULL)

flashArd: images
	@printf "  FLASH   $(NAME).srec\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) \
		       -f $(OPENOCD_FLASHER) \
		       -f $(OPENOCD_TARGET) \
		       -c "init" -c "reset halt" \
		       -c "flash write_image erase $(NAME).srec" \
		       -c "reset" \
		       -c "shutdown" $(NULL)


.PHONY: images clean

