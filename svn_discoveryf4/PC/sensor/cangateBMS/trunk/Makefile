# Makefile - PC
# 11/28/2013

# Name of the .c file with 'main' as well as output file name
NAME		= cangateBMS

# Current directory
SRCDIR=.

LIBCOMMONALL    =$(SRCDIR)/../../../../common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall -Wstrict-prototypes \


# Libraries (.a)
LDFLAGS         =   -lz -lpthread -lm -lncurses

# List of files to be compiled
OBJS = $(NAME).o \
commands.o \
cmd_a.o \
cmd_b.o \
cmd_c.o \
cmd_C.o \
cmd_d.o \
cmd_e.o \
cmd_E.o \
cmd_f.o \
cmd_h.o \
cmd_i.o \
cmd_j.o \
cmd_k.o \
cmd_l.o \
cmd_m.o \
cmd_n.o \
cmd_p.o \
cmd_q.o \
cmd_r.o \
cmd_s.o \
cmd_t.o \
cmd_w.o \
cmd_v.o \
sockclient.o \
timer_thread.o \
var_size_chk.o \
PC_gateway_comm.o \
USB_PC_gateway.o \



canldr: $(OBJS)
	gcc -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(NAME)\
	rm -f *.o

.PHONY: clean


