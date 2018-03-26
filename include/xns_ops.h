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
//////symbols and gensyms
/*
 * Create or find a symbol with the given name.  As symbols are unique in a
 * given VM (namewise), this should be the only function to create symbols!
 */
struct xns_object *xns_intern(xns_vm *vm, const char *name);
/*
 * Generate a "GENSYM" (generated symbol).  These are used when one needs a
 * symbol but needs it to be completely unique (ex: to avoid variable capture
 * in a macro).
 */
struct xns_object *xns_gensym(xns_vm *vm);
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
bool xns_eq(struct xns_object *a, struct xns_object *b);
/*
 * Is the given object eq to NIL?
 * -- xns_nil should be the same is (EQ OBJ 'NIL) unless obj==NULL 
 * (Note: NIL is the Lisp equivalent of NULL -- it is the empty list, a symbol named 'NIL, and false)
 */
bool xns_nil(xns_object *obj);
// environment
struct xns_object *xns_make_env(xns_vm *vm, xns_object *parent);
struct xns_object *xns_assoc(xns_object *env, xns_object *sym);
struct xns_object *xns_set(xns_object *env, xns_object *sym, xns_object *value);
// Cons Cells
struct xns_object *xns_car(struct xns_object *obj);
struct xns_object *xns_cdr(struct xns_object *obj);
struct xns_object *xns_cons(struct xns_vm *vm, struct xns_object *car, struct xns_object *cdr);
struct xns_object *xns_nreverse(struct xns_object *list);
size_t xns_len(struct xns_object *list);
struct xns_object *xns_pair(struct xns_object *x, struct xns_object *y);
struct xns_object *xns_append(struct xns_object *x, struct xns_object *y);
////// oh no 
inline struct xns_object *xns_caar(struct xns_object *obj){return xns_car(xns_car(obj));}
inline struct xns_object *xns_cadr(struct xns_object *obj){return xns_car(xns_cdr(obj));}
inline struct xns_object *xns_cdar(struct xns_object *obj){return xns_cdr(xns_car(obj));}
inline struct xns_object *xns_cddr(struct xns_object *obj){return xns_cdr(xns_cdr(obj));}
/*inline struct xns_object *xns_caaar(struct xns_object *obj)
struct xns_object *xns_caadr(struct xns_object *obj);
struct xns_object *xns_cadar(struct xns_object *obj);
struct xns_object *xns_caddr(struct xns_object *obj);
struct xns_object *xns_cdaar(struct xns_object *obj);
struct xns_object *xns_cdadr(struct xns_object *obj);
struct xns_object *xns_cddar(struct xns_object *obj);
struct xns_object *xns_cdddr(struct xns_object *obj);*/
// numbers
//////////////TODO
// constructors
struct xns_object *xns_make_fixnum(struct xns_vm *vm, long value);
struct xns_object *xns_make_double(struct xns_vm *vm, double value);
struct xns_object *xns_make_primitive(struct xns_vm *vm, xns_primitive value);
struct xns_object *xns_make_string(struct xns_vm *vm, char *value);
struct xns_object *xns_make_function(struct xns_vm *vm, struct xns_object *params, struct xns_object *body, struct xns_object *env);
struct xns_object *xns_make_macro(struct xns_vm *vm, struct xns_object *params, struct xns_object *body, struct xns_object *env);
//struct xns_object *xns_make_fixnums(struct xns_vm *vm, long value); // this is a template
#endif //XNS_OPS_H