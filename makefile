CC=gcc
CFLAGS=-Wall
ODIR=obj

LIBS=-lrt -lm

DEPS = pingpong.h queue.h datatypes.h

OBJ = pingpong.c queue.c pingpong-ipc.c pingpong-msg.c

ENT12 = $(OBJ) pingpong-mqueue.c


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

entrega12: $(ENT12)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
