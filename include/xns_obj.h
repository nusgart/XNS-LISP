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

typedef enum xns_type{
  // not a valid type -- must have been uninitialized
  XNS_INVL=0,
  // a cons Cell -- points to 
  XNS_CONS,
  // symbol -- compare eq
  XNS_SYMBOL,
  // a primitive operation (implemented in C code)
  XNS_PRIMITIVE,
  // a function (implemented in XNS Lisp)
  XNS_FUNCTION,
  // a macro -- UNIMPLEMENTED
  XNS_MACRO,
  // an environment frame: this is a map from symbols to actual xns_object values
  XNS_ENV,
  // an object that has been moved to a new heap -- should only EVER be present during a GC
  XNS_MOVED,
  // a fixed-size integer (in this implementation, it is implemented as long)
  XNS_FIXNUM,
  // an integer -- UNIMPLEMENTED
  XNS_INTEGER,
  // a doub
  XNS_DOUBLE,
  // a C-style null terminated string (but size is used as a sanity check) 
  XNS_STRING,
  // a rational number -- UNIMPLEMENTED
  XNS_RATIONAL,
  // a generated symbol -- this can only be returned by a 
  XNS_GENSYM,
  // a pointer to a foreign function -- UNIMPLEMENTED
  XNS_FOREIGN_FUNC,
  // a pointer to a foreign object
  XNS_FOREIGN_PTR,
  // allow normal C code to interact with xns_objects -- unimplemented
  XNS_HANDLE
}  xns_type;

// Represents an XNS-Object
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
        struct xns_object *car;
        struct xns_object *cdr;
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
        struct xns_object *args;
        struct xns_object *body;
        struct xns_object *env;
    };
    // environment frame
    struct{
        struct xns_object *vars;
        struct xns_object *parent;
    };
    // integer -- unimplemented -- will change
    void *integer_implementation;
    // double
    double dval;
    // string
    char *string;
    // rational -- unimplemented -- will change
    void *rational_implementation;
    // foreign function pointer -- unimplemented but this probably won't change
    struct{
        void *(*foreign_fcn)();
        // will somehow hold the signature for type safety??
        struct xns_object *signature; 
    };
    // handle or moved
    xns_object *ptr;
    // foreign pointer -- unimplemented but final
    void *foreign_pointer;
  };
};

// allocates space for an XNS-Object
xns_object *xns_alloc_object(struct xns_vm *vm, enum xns_type type, size_t size);
// gives out an xns_handle -- this is an object that normal C code can handle
xns_object *xns_get_handle(struct xns_vm *vm, struct xns_object *obj);
// destroy a handle -- until handles are destroyed
void xns_destroy_handle(struct xns_vm *vm, struct xns_object *handle);

#endif //XNS_OBJ_H