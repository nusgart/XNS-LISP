### Copyright 2018 - Nicholas Nusgart
### XNS-Lisp Makefile
CFLAGS=-std=gnu11 -O0 -g -Iinclude -Wall -Wextra
OBJECTS=src/main.o src/xns_heap.o src/xns_ops.o src/xns_vm.o src/xns_io.o

.PHONY: all clean

all: xns_lisp

xns_lisp: $(OBJECTS)
	$(CC) -o xns_lisp $(OBJECTS) -g -lm

clean:
	$(RM) $(OBJECTS) xns_lisp
