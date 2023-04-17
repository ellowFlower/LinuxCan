CC=gcc
CFLAGS=-I.
DEPS=can_demo.h
OBJ = can_demo.o canreceive.o cantransmit.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

can_demo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
