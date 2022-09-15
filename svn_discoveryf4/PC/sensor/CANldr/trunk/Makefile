# Makefile - PC
# 11/28/2013

# Name of the .c file with 'main' as well as output file name
NAME		= CANldr

# Current directory
SRCDIR=.

LIBCOMMONALL    =$(SRCDIR)/../../../../common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall -Wstrict-prototypes \
-I../../../../sw_discoveryf4/trunk/lib/libmiscstm32f4 \
-I../../../../sw_discoveryf4/trunk/lib \
-I$(LIBCOMMONALL) \


# Libraries (.a)
LDFLAGS         = -L../../../../common_all/trunk/PC -lcommon_all -lz -lpthread -lrt

# List of files to be compiled
OBJS = $(NAME).o \
commands.o \
cmd_c.o \
download.o\
sockclient.o \
timer_thread.o \
crc-32_nib.o \



canldr: $(OBJS)
	gcc -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(NAME)\
	rm -f *.o

.PHONY: clean


