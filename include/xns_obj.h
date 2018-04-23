/* 
  Copyright 2018- Nicholas Nusgart, All Rights Reserved

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#ifndef XNS_OBJ_H
#define XNS_OBJ_H

#include "xns_common.h"
/**
 * xns_type describes the type of an xns_object.
 */
typedef enum xns_type{
  // not a valid type -- the object wasn't initialized or is garbage data!
  XNS_INVL=0,
  // a cons Cell -- this is a pair of two xns_objects (pointerwise, as always) 
  XNS_CONS,
  // Symbols are named atoms that can be compared with EQ (there is only symbol with a given name). 
  XNS_SYMBOL,
  // A primitive operation (implemented in C code).
  XNS_PRIMITIVE,
  // A normal function (implemented in XNS Lisp code).
  XNS_FUNCTION,
  // A macro -- UNIMPLEMENTED
  XNS_MACRO,
  // an environment frame: this is a map from symbols to actual xns_object values
  XNS_ENV,
  // an object that has been moved to a new heap -- should only EVER be present during a GC
  XNS_MOVED,
  // a fixed-size integer (in this implementation, it is implemented as long)
  XNS_FIXNUM,
  // an integer (any size) -- UNIMPLEMENTED
  XNS_INTEGER,
  // a doub
  XNS_DOUBLE,
  // a C-style null terminated string (but size is used as a sanity check): stored inline
  XNS_STRING,
  // a rational number -- UNIMPLEMENTED
  XNS_RATIONAL,
  // a generated symbol -- these are NOT interned -- see ususal lisp expanation
  XNS_GENSYM,
  // a pointer to a foreign function -- UNIMPLEMENTED
  XNS_FOREIGN_FUNC,
  // a pointer to a foreign object
  XNS_FOREIGN_PTR,
  // allow normal C code to interact with xns_objects -- partially implemented
  XNS_HANDLE,
  // an array of xns_objects
  XNS_ARRAY
}  xns_type;

/**
 * This structure represents an XNS object in list structured memory (except
 * for XNS_HANDLEs, which are convience )
 */
struct xns_object{
  //// OBJECT HEADER
  // the object's type
  enum xns_type type;
  // object identifier -- if this proves to be too small, then 
  unsigned int object_id;
  // the size of the object
  size_t size;
  // pointer to the parent VM
  struct xns_vm *vm;
  //// OBJECT CONTENTS
  union{
    // cons cell
    struct{
        xns_obj car;
        xns_obj cdr;
    };
    // symbol -- 
    struct{
        size_t symid;
        char symname[];
    };
    // primitive operation
    xns_primitive primitive;
    // fixnum
    long fixnum;
    // function / macro
    struct{
        xns_obj args;
        xns_obj body;
        xns_obj env;
    };
    // environment frame
    struct{
        xns_obj vars;
        xns_obj parent;
    };
    // integer -- unimplemented -- will change
    void *integer_implementation;
    // double
    double dval;
    // string
    struct{
        size_t len;
        char string[];
    };
    // array
    struct {
        size_t length;
        xns_obj array[];
    };
    // rational -- unimplemented -- will change
    void *rational_implementation;
    // foreign function pointer -- unimplemented but this probably won't change
    struct{
        struct xns_object *(*foreign_fcn)(xns_obj args, xns_obj env);
        // will somehow hold the signature for type safety??
        xns_obj signature; 
    };
    // handle or moved
    xns_obj ptr;
    // foreign pointer -- unimplemented but final
    void *foreign_pointer;
  };
};

// allocates space for an XNS-Object
xns_object *xns_alloc_object(struct xns_vm *vm, enum xns_type type, size_t size);
// gives out an xns_handle -- this is an object that normal C code can handle
xns_object *xns_get_handle(struct xns_vm *vm, xns_obj obj);
// destroy a handle -- until handles are destroyed
void xns_destroy_handle(struct xns_vm *vm, xns_obj handle);

#endif //XNS_OBJ_H