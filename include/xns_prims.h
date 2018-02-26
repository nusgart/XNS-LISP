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
#ifndef XNS_PRIMS_H
#define XNS_PRIMS_H
// defines some common operations on objects
#include "xns_common.h"
// Internal function -- called on VM startup
void xns_register_primops(struct xns_vm *vm);
//symbols and gensyms
xns_object *xns_prim_eq     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_null   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_quote    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
// environment
//xns_object *xns_make_env(xns_vm *vm, xns_object *parent);
xns_object *xns_prim_set    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_assoc  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
// control
xns_object *xns_prim_let    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cond   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_lambda (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_label  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
// Cons Cells
xns_object *xns_prim_car    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cdr    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cons   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_pair   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_append (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
//Predicates
xns_object *xns_prim_atom   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_and    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_or     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_not    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);

//numbers
xns_object *xns_prim_plus   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_minus  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_mult   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_divide (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_expt   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);

#endif //XNS_PRIMS_H