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
void xns_register_primops   (struct xns_vm *vm);
// exit the vm
xns_object *xns_prim_exit   (struct xns_vm *vm, xns_obj env, xns_obj args);
//symbols and gensyms
xns_object *xns_prim_eq     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_null   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_quote  (struct xns_vm *vm, xns_obj env, xns_obj args);
// environment
xns_object *xns_prim_set    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_setf   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_define (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_assoc  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_eval   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_apply  (struct xns_vm *vm, xns_obj env, xns_obj args);
// type
xns_object *xns_prim_typeof (struct xns_vm *vm, xns_obj env, xns_obj args);
// control
xns_object *xns_prim_progn  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_let    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cond   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_while  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_lambda (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_mlambda (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_labels (struct xns_vm *vm, xns_obj env, xns_obj args);
// Cons Cells
xns_object *xns_prim_car    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cdr    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_setcdr (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_setcar (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cons   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_pair   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_append (struct xns_vm *vm, xns_obj env, xns_obj args);
//Predicates
xns_object *xns_prim_atom   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_and    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_or     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_not    (struct xns_vm *vm, xns_obj env, xns_obj args);
///////////////////////////File Operations: TODO
xns_object *xns_prim_load   (struct xns_vm *vm, xns_obj env, xns_obj args);
///////////////////////////numbers
/// arith ops
xns_object *xns_prim_plus   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_minus  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_mult   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_divide (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_lesser (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_greater (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_equal  (struct xns_vm *vm, xns_obj env, xns_obj args);
/// basic ops
xns_object *xns_prim_isnan  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_abs    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_mod    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_sqrt   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cbrt   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_hypot  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_expt   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_exp    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_pow    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_log    (struct xns_vm *vm, xns_obj env, xns_obj args);
/// advanced ops
xns_object *xns_prim_erf    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_erfc   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_gamma  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_lgamma (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_j0     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_j1     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_jn     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_y0     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_y1     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_yn     (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_expm1  (struct xns_vm *vm, xns_obj env, xns_obj args);
/// rounding
xns_object *xns_prim_floor  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_ceil   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_round  (struct xns_vm *vm, xns_obj env, xns_obj args);

/// trig ops
xns_object *xns_prim_sin    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cos    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_tan    (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_asin   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_acos   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_atan   (struct xns_vm *vm, xns_obj env, xns_obj args);
/// hyperbolic trig
xns_object *xns_prim_sinh   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_cosh   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_tanh   (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_asinh  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_acosh  (struct xns_vm *vm, xns_obj env, xns_obj args);
xns_object *xns_prim_atanh  (struct xns_vm *vm, xns_obj env, xns_obj args);
#endif //XNS_PRIMS_H