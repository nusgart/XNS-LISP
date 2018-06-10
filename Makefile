### Copyright 2018 - Nicholas Nusgart
### XNS-Lisp Makefile
CFLAGS=-std=gnu11 -O4 -g -Iinclude -Wall -Wextra -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1
PRIMOBJ=src/prims/specials.o src/prims/math.o src/prims/io.o src/prims/arith.o src/prims/logic.o src/prims/type.o src/prims/array.o
OBJECTS=src/main.o src/xns_heap.o src/xns_ops.o src/xns_vm.o src/xns_io.o $(PRIMOBJ) src/xns_eval.o

.PHONY: all clean

all: xns_lisp

src/typestr.inc: include/xns_obj.h Makefile
	awk '/typedef enum xns_type/{flag=1;next}/\}/{flag=0}flag' include/xns_obj.h\
		| grep -v '^\s*//' | cut -d '=' -f 1 | tr -d ' ,' | (echo "char *xns_type_strs[] = { ";\
		sed 's/$$/",/g' | sed 's/^/  "/g'; echo "};") > src/typestr.inc

src/prims/types.inc: src/typestr.inc
	sed 's/XNS_/:/g' <src/typestr.inc | sed 's/xns_type_strs/xns_types/g' > src/prims/types.inc

src/prims/primops.inc: include/xns_prims.h Makefile
	for i in `grep xns_object include/xns_prims.h  | cut -d ' ' -f 2 | tr -d '*'` ;do\
	       echo "    "rp\(vm, \"`echo -n $$i | sed s/xns_prim_//g`\", $$i\)\; ;\
	done > src/prims/primops.inc

src/prims/specials.o: src/prims/primops.inc
src/prims/type.o: src/prims/types.inc

src/xns_io.o: src/typestr.inc

$(OBJECTS): Makefile

xns_lisp: $(OBJECTS)
	$(CC) -o xns_lisp $(OBJECTS) -g -lm

clean:
	$(RM) $(OBJECTS) xns_lisp src/prims/primops.inc src/typestr.inc
