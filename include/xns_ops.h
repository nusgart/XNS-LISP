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
//symbols and gensyms
xns_object *xns_intern(xns_vm *vm, char *name);
xns_object *xns_gensym(xns_vm *vm);
bool xns_eq(xns_object *a, xns_object *b);
bool xns_nil(xns_object *obj);
// environment
xns_object *xns_make_env(xns_vm *vm, xns_object *parent);
xns_object *xns_find(xns_object *env, xns_object *sym);
xns_object *xns_assoc(xns_object *env, xns_object *sym, xns_object *value);
// Cons Cells
xns_object *xns_car(xns_object *obj);
xns_object *xns_cdr(xns_object *obj);
xns_object *xns_cons(xns_vm *vm, xns_object *car, xns_object *cdr);
#endif //XNS_OPS_H