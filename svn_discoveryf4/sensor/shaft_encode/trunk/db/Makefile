# Sources

# Name of the .c file with 'main' as well as output file name
NAME	= encoder_tables_all

# Linker file
# Note: ENTRY in .ld file is omitted so it skips looking for label
LDFILE = can_unit.ld

# Current directory
CURDIR=.
SRCS =
SRCS += encoder_lo_param.c
SRCS += encoder_hi_param.c
SRCS += unit_cmd_canid.c

###################################################
# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump

CFLAGS  = -std=gnu99 -g -Os -Wall -Wextra
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4
CFLAGS +=  -fno-common -Wstrict-prototypes 
CFLAGS += -mfp16-format=alternative

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################

# Navigation to libraries specific to the f4 and discoveryf4
LIBF4D		= $(CURDIR)/../../../../sw_discoveryf4/trunk

# Library of common to all processors
LIBSVNCOMMON	= $(CURDIR)/../../../../../svn_common/trunk

# Includes--our stuff
CFLAGS += -I$(LIBSVNCOMMON)
CFLAGS += -I$(LIBSVNCOMMON)/db
CFLAGS += -I../$(CURDIR)


# Library paths--our stuff
LIBPATHS += -L$(LIBF4D)/lib/libdiscoveryf4
LIBPATHS += -L$(LIBF4D)/lib/libmiscstm32f4 
LIBPATHS += -L$(LIBF4D)/lib/libusartstm32f4
LIBPATHS += -L$(LIBSVNCOMMON)/f4

# Libraries to link
#   our stuff
#LIBS	+= -ldiscoveryf4 -lmiscstm32f4 -lusartstm32f4 -lcommon_allf4  -ldiscoveryf4

LDFLAGS	 = -T$(LDFILE)

OBJS = $(SRCS:.c=.o)

# Paths for openocd and automatic flashing
OPENOCD_BASE	= /usr/share
OPENOCD		= /usr/bin/openocd
OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/openocd/scripts
OPENOCD_FLASHER	= $(OPENOCD_SCRIPTS)/interface/stlink-v2.cfg
OPENOCD_BOARD	= $(OPENOCD_SCRIPTS)/board/stm32f4discovery.cfg

###################################################

.PHONY: proj

all: proj
	$(SIZE) $(NAME).elf

proj: 	$(NAME).elf

$(NAME).elf: $(SRCS)
	$(CC) $(CFLAGS)  $^ -o $@ $(LIBPATHS) $(LIBS) $(LDFLAGS) -Wl,-Map=$(NAME).map
	$(OBJCOPY) -O ihex   $(NAME).elf $(NAME).hex
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin
	$(OBJCOPY) -O srec   $(NAME).elf $(NAME).srec
	$(OBJDUMP) -S $(NAME).elf > $(NAME).list

$(NAME): $(OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)

clean:

clean:
	rm -f *.o
	rm -f $(NAME).elf
	rm -f $(NAME).hex
	rm -f $(NAME).bin
#	$(MAKE) clean -C lib # Remove this line if you don't want to clean the libs as well
	
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

