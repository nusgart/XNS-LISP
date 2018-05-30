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

#pragma once
#ifndef XNS_OPS_H
#define XNS_OPS_H
// defines some common operations on objects
#include "xns_common.h"
////// general objects
/*
 * Hash the given object.  This hash algorithm is NOT cryptographically secure right now, however, that might change.
 * This only works on simple objects right now.
 */
unsigned long xns_hash(xns_obj obj);
//////symbols and gensyms
/*
 * Create or find a symbol with the given name.  As symbols are unique in a
 * given VM (namewise), this should be the only function to create symbols!
 */
xns_object *xns_intern(xns_vm *vm, const char *name);
/*
 * Generate a "GENSYM" (generated symbol).  These are used when one needs a
 * symbol but needs it to be completely unique (ex: to avoid variable capture
 * in a macro).
 */
xns_object *xns_gensym(xns_vm *vm);
/*
 * Are the two objects given pointerwise equal or the same symbol? EQ ONLY
 * WORKS ON SYMBOLS! WHETHER TO NON-SYMBOL OBJECTS ARE EQ IS UNSPECIFIED!
 * (EQ 1 1) or even (eq a a) MAY EVALUATE TO EITHER 'T OR 'NIL, POSSIBLY BOTH
 * DURING THE EXECUTION OF THE SAME PROGRAM!
 * 
 * Note: Pointerwise equality should be enough to compare two symbols but the
 * symbol id is compared just in case xns_eq is called during the middle of a
 * GC, or symbols are accidentally copied. 
 */
bool xns_eq(xns_obj a, xns_obj b);
/*
 * Is the given object eq to NIL?
 * -- xns_nil should be the same is (EQ OBJ 'NIL) unless obj==NULL 
 * (Note: NIL is the Lisp equivalent of NULL -- it is the empty list, a symbol named 'NIL, and false)
 */
bool xns_nil(xns_obj obj);
// environment
xns_object *xns_make_env(xns_vm *vm, xns_obj parent);
xns_object *xns_assoc(xns_obj env, xns_obj sym);
xns_object *xns_set(xns_obj env, xns_obj sym, xns_obj value);
xns_object *xns_bind(xns_obj env, xns_obj sym, xns_obj value);
// Cons Cells
xns_object *xns_car(xns_obj obj);
xns_object *xns_cdr(xns_obj obj);
xns_object *xns_cons(struct xns_vm *vm, xns_obj car, xns_obj cdr);
xns_object *xns_nreverse(xns_obj list);
size_t xns_len(xns_obj list);
xns_object *xns_pair(xns_obj x, xns_obj y);
xns_object *xns_append(xns_obj x, xns_obj y);
////// oh no 
inline xns_object *xns_caar(xns_obj obj){return xns_car(xns_car(obj));}
inline xns_object *xns_cadr(xns_obj obj){return xns_car(xns_cdr(obj));}
inline xns_object *xns_cdar(xns_obj obj){return xns_cdr(xns_car(obj));}
inline xns_object *xns_cddr(xns_obj obj){return xns_cdr(xns_cdr(obj));}
/*inline xns_object *xns_caaar(xns_obj obj)
xns_object *xns_caadr(xns_obj obj);
xns_object *xns_cadar(xns_obj obj);
xns_object *xns_caddr(xns_obj obj);
xns_object *xns_cdaar(xns_obj obj);
xns_object *xns_cdadr(xns_obj obj);
xns_object *xns_cddar(xns_obj obj);
xns_object *xns_cdddr(xns_obj obj);*/
// numbers
//////////////TODO
// constructors
xns_object *xns_make_fixnum(struct xns_vm *vm, long value);
xns_object *xns_make_double(struct xns_vm *vm, double value);
xns_object *xns_make_primitive(struct xns_vm *vm, xns_primitive value);
xns_object *xns_make_string(struct xns_vm *vm, char *value);
xns_object *xns_make_function(struct xns_vm *vm, xns_obj params, xns_obj body, xns_obj env);
xns_object *xns_make_macro(struct xns_vm *vm, xns_obj params, xns_obj body, xns_obj env);
xns_object *xns_make_array(struct xns_vm *vm, size_t len);
//xns_object *xns_make_fixnums(struct xns_vm *vm, long value); // this is a template
///////conversion
xns_object *xns_to_fixnum(struct xns_vm *vm, xns_obj value);
xns_object *xns_to_integer(struct xns_vm *vm, xns_obj value);
xns_object *xns_to_real(struct xns_vm *vm, xns_obj value);
xns_object *xns_to_double(struct xns_vm *vm, xns_obj value);
#endif //XNS_OPS_H