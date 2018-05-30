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
// XNS-Lisp primitive array functions 693f7257
#include "xns_lisp.h"

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)

xns_object *xns_prim_hash   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "Wrong number of arguments passed to length", args);
    }
    xns_obj arg = eval(args->car, env);
    unsigned long hash = xns_hash(arg);
    return xns_make_fixnum(vm, (long)hash);
}

xns_object *xns_prim_length (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "Wrong number of arguments passed to length", args);
    }
    xns_obj arg = eval(args->car, env);
    size_t len = xns_len(arg);
    return xns_make_fixnum(vm, len);
}

xns_object *xns_prim_makearray(struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "Wrong number of arguments passed to makearray", args);
    }
    xns_obj arg = eval(args->car, env);
    if (arg->type != XNS_FIXNUM){
        vm->error(vm, "Non-fixnum type passed to makearray", arg);
        return vm->NIL;
    }
    if (arg->fixnum < 0){
        vm->error(vm, "Negative length passed to makearray", arg);
        return vm->NIL;
    }
    size_t len = arg->fixnum;
    xns_obj ret = xns_make_array(vm, len);
    // initialize array with NIL's
    for (int i = 0; i < arg->fixnum; i++) {
        ret->array[i] = vm->NIL;
    }
    return ret;
}

xns_object *xns_prim_aref   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        vm->error(vm, "Wrong number of arguments passed to aref", args);
    }
    R(env); R(args);
    xns_obj arr = eval(args->car, env);
    R(arr);
    xns_obj idx = eval(args->cdr->car, env);
    U(env); U(args); U(arr);
    if (idx->type != XNS_FIXNUM) {
        vm->error(vm, "aref index must be a fixnum", idx);
        return vm->NIL;
    }
    if(arr->type != XNS_ARRAY && arr->type != XNS_STRING){
        vm->error(vm, "aref array must be an array or a string", arr);
        return vm->NIL;
    }
    long index = idx->fixnum;
    if (index < 0 || (size_t)index > arr->len){
        char *desc;
        asprintf(&desc,"Array Index %ld is out of bounds (length=%lu).", index, arr->len);
        vm->error(vm, desc, args);
        free(desc);
        return vm->NIL;
    }
    if(arr->type == XNS_STRING){
        return xns_make_fixnum(vm, arr->string[index]);
    }
    return arr->array[index];

}
xns_object *xns_prim_setf$aref (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 3){
        vm->error(vm, "Wrong number of arguments passed to setf$aref", args);
    }
    R(env); R(args);
    xns_obj arr = eval(args->car, env);
    R(arr);
    xns_obj idx = eval(args->cdr->car, env);
    
    // type checking
    if (idx->type != XNS_FIXNUM) {
        vm->error(vm, "setf$aref index must be a fixnum", idx);
        return vm->NIL;
    }
    if(arr->type != XNS_ARRAY && arr->type != XNS_STRING){
        vm->error(vm, "setf$aref array must be an array or a string", arr);
        return vm->NIL;
    }
    long index = idx->fixnum;
    if (index < 0 || (size_t)index > arr->len){
        char *desc;
        asprintf(&desc,"Array Index %ld is out of bounds (length=%lu).", index, arr->len);
        vm->error(vm, desc, args);
        free(desc);
        return vm->NIL;
    }
    // perform setting
    xns_obj val = eval(args->cdr->cdr->car, env);
    if (arr->type == XNS_STRING){
        val = xns_to_fixnum(vm, val);
        arr->string[index] = (char)val->fixnum;
    } else {
        arr->array[index] = val;
    }
    U(env); U(args); U(arr);
    return val;
}