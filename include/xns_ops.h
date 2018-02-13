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
struct xns_object *xns_intern(xns_vm *vm, char *name);
/*
 * Generate a "GENSYM" (generated symbol).  These are used when one needs a
 * symbol but needs it to be completely unique (ex: to avoid variable capture
 * in a macro).
 */
struct xns_object *xns_gensym(xns_vm *vm);
/*
 *  Are the two objects given pointerwise equal or the same symbol?
 */
bool xns_eq(xns_object *a, xns_object *b);
/*
 * Is the given object NIL?
 */
bool xns_nil(xns_object *obj);
// environment
struct xns_object *xns_make_env(xns_vm *vm, xns_object *parent);
struct xns_object *xns_find(xns_object *env, xns_object *sym);
struct xns_object *xns_assoc(xns_object *env, xns_object *sym, xns_object *value);
// Cons Cells
struct xns_object *xns_car(xns_object *obj);
struct xns_object *xns_cdr(xns_object *obj);
struct xns_object *xns_cons(xns_vm *vm, xns_object *car, xns_object *cdr);

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