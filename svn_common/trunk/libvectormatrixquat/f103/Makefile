#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/02/2015
#* Description        : Build library common to PC and STM32--for PC
#*******************************************************************************/ 
LIBNAME		= libvmq


# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
CC		= $(PREFIX)-gcc
LD		= $(PREFIX)-ld
OBJCOPY		= $(PREFIX)-objcopy
OBJDUMP		= $(PREFIX)-objdump
AR		= $(PREFIX)-ar


# Define path to source files.
VPATH=../

# Current directory
THISDIR=.

# Library directory path--common includes
LIBCOMMONALL = $(VPATH)
LIBPODLIB    = $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/lib

# Library directory path prefix--svn_pod library
LIBPODLIB	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/devices
# Library directory path--svn_pod library 
LIBOPENSTM32	= $(LIBPODLIB)/libopenstm32

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(THISDIR)/../../../sw_f103/trunk/lib
# Library directory path--svn_sensor, f103 library
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Path to lc lm lgcov lgcc and other good libraries
LCC = $(ARM_PREFIX)/arm-none-eabi/lib/armv7-m
LOTHER = $(ARM_PREFIX_GCCLIB)2


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra 
CFLAGS += -I$(LIBF103)/lib/libvectormatrixquat
CFLAGS += -I$(LIBCOMMONALL) 
CFLAGS += -I$(LIBOPENSTM32) 
CFLAGS += -I$(LIBSENSORMISC)
CFLAGS += -Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

# Routines to be compiled into the library
# ### OBJS is 'export'ed with object files in 'Makefile' in directory above.


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

