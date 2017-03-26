#/******************** (C) COPYRIGHT 2010 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 06/04/2011 deh
#* Description        : Build library: libudevicesstm32.a
#*******************************************************************************/ 
LIBNAME		= libdevicesstm32

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
CC		= $(PREFIX)-gcc
LD		= $(PREFIX)-ld
OBJCOPY		= $(PREFIX)-objcopy
OBJDUMP		= $(PREFIX)-objdump
AR		= $(PREFIX)-ar

# Current directory
SRCDIR=.
# Library directory path
LIBDIR		= $(SRCDIR)/../lib
LIBDEV		= $(SRCDIR)/../devices
GPIODIR		= $(SRCDIR)/../gpiotest

# Library directory path
LIBDIR		= $(SRCDIR)/../lib/

# Library directory path
LIBDEVICESSTM32	= $(LIBDEV)
LIBMISCSTM32	= $(LIBDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBDIR)/libusartstm32
LIBSUPPORTSTM32	= $(LIBDIR)/libsupportstm32

# Path to lc lm lgcov lgcc and other good libraries
LCC = $(ARM_PREFIX)/arm-none-eabi/lib/armv7-m
LOTHER = $(ARM_PREFIX_GCCLIB)

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS	= -Os -g -Wall -Wextra  -I$(LIBDEV) -I$(LIBDIR) -Iinclude -fno-common 
CFLAGS += -mcpu=cortex-m3 -mthumb -Wstrict-prototypes 
CFLAGS += -I$(LIBOPENSTM32) 
CFLAGS += -I$(LIBMISCSTM32) 
CFLAGS += -I$(LIBSUPPORTSTM32) 

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
32KHz.o \
spi1ad7799.o \
ad7799_comm.o \
adcpod.o \
gps_1pps.o \
hwcrc.o \
pinconfig.o \
pinconfig_all.o \
PODpinconfig.o \
pwrctl.o \
sdcard.o \
sdcard_ll.o \
spi2sdcard.o \
Tim2_pod.o \
Tim3_pod.o \
 \





# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

all: $(LIBNAME).a

$(LIBNAME).a: $(OBJS)
	@printf "  AR      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o
	@printf "  CLEAN   $(LIBNAME).a\n"
	$(Q)rm -f $(LIBNAME).a

.PHONY: clean

