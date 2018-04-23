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
// XNS-Lisp primitive operator implementation
// most of the primitives in this file are specials/fundamental operators
// TODO finish refactoring to make that all of the primitives in this file
// TODO distinguish langauge primitives from builtin functions
#include "xns_lisp.h"

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)

xns_object *xns_prim_and    (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj o = vm->NIL;
    while(!xns_nil(args)){
        o = eval(args->car, env);
        if(xns_nil(o)){
            U(env);
            U(args);
            return vm->NIL;
        }
        args = args->cdr;
    }
    U(env);
    U(args);
    return o;
}
xns_object *xns_prim_or     (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj o = vm->NIL;
    while(!xns_nil(args)){
        o = eval(args->car, env);
        if(!xns_nil(o)){
            U(env);
            U(args);
            return o;
        }
        args = args->cdr;
    }
    U(env);
    U(args);
    return vm->NIL;
}
xns_object *xns_prim_not    (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "not called with too many arguments", args);
        return vm->NIL;
    }
    xns_obj expr = eval(args->car, env);
    if(xns_nil(expr)){
        return vm->T;
    }
    return vm->NIL;
}