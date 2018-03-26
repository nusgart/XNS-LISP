### Copyright 2018 - Nicholas Nusgart
### XNS-Lisp Makefile
CFLAGS=-std=gnu11 -O0 -g -Iinclude -Wall -Wextra
PRIMOBJ=src/xns_prims.o src/xns_math_prims.o src/xns_prims_io.o
OBJECTS=src/main.o src/xns_heap.o src/xns_ops.o src/xns_vm.o src/xns_io.o $(PRIMOBJ) src/xns_eval.o

.PHONY: all clean

all: xns_lisp


src/primops.inc: include/xns_prims.h
	for i in `grep xns_object include/xns_prims.h  | cut -d ' ' -f 2 | tr -d '*'` ;do\
	       echo "    "rp\(vm, \"`echo -n $$i | sed s/xns_prim_//g`\", $$i\)\; ;\
	done > src/primops.inc

src/xns_prims.o: src/primops.inc


xns_lisp: $(OBJECTS)
	$(CC) -o xns_lisp $(OBJECTS) -g -lm

clean:
	$(RM) $(OBJECTS) xns_lisp src/primops.inc
