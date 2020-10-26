CC=gcc
CFLAGS=-Wall

ODIR=obj


LIBS=-lrt -lm

DEPS = pingpong.h queue.h datatypes.h

OBJ = pingpong.c queue.c

ENT6a = $(OBJ) pingpong-contab.c

ENT6b = $(OBJ) pingpong-contab-prio.c


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

contab: $(ENT6a)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

prio: $(ENT6b)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

