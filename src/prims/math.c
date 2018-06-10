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
// XNS-Lisp primitive mathematical functions
// these are really builtins
#include "xns_lisp.h"
#include <math.h>
#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)
/// basic ops
xns_object *xns_prim_isnan  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "isnan called with an incorrect number of arguments", args);
        return vm->T;
    }
    R(args); R(env);
    xns_obj arg = xns_to_double(vm, eval(args->car, env));
    if(xns_nil(arg)){
        vm->error(vm, "mod called with a non-numeric argument", args);
        return vm->T;
    }
    U(env); U(args);
    bool res = isnan(arg->dval);
    if(res) return arg;
    return vm->NIL;
}
xns_object *xns_prim_expt  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        vm->error(vm, "expt called with an incorrect number of arguments", args);
        return vm->T;
    }
    R(args); R(env);
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));
    R(arg1);
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));
    if(xns_nil(arg1)||xns_nil(arg2)) return vm->T;
    U(args); U(env); U(arg1);
    return xns_make_double(vm, pow(arg1->dval, arg2->dval));
}
xns_object *xns_prim_mod  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        vm->error(vm, "mod called with an incorrect number of arguments", args);
        return vm->T;
    }
    R(args); R(env);
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));
    R(arg1);
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));
    if(xns_nil(arg1) || xns_nil(arg2)){
        return vm->T;
    }
    U(env); U(args);
    double res = fmod(arg1->dval, arg2->dval);
    return xns_make_double(vm, res);
}

#define M(x) xns_object * xns_prim_ ## x (struct xns_vm *vm, xns_obj env, xns_obj args){\
    if (xns_len(args) != 1)return vm->T;\
    R(args); R(env);\
    xns_obj arg = xns_to_double(vm, eval(args->car, env));\
    U(args); U(env);\
    if(xns_nil(arg)) return vm->T;\
    double res = x (arg->dval);\
    return xns_make_double(vm, res);}
M(abs);
M(sqrt);
M(expm1);
M(cbrt);
M(log);
M(erf);
M(exp);
M(erfc);
#if defined(__GNUC__) && defined(__linux__) && !defined(__GLIBC__) && !defined(__clang__)
// fix for musl -- this is somewhat excessive, but musl has no define.
static inline double gamma(double x){
    return __builtin_gamma(x);
}
#endif
M(gamma);
M(lgamma);
M(j0);
M(j1);
M(y0);
M(y1);
M(floor);
M(ceil);
M(round);
M(sin);
M(cos);
M(tan);
M(asin);
M(acos);
M(atan);
M(sinh);
M(cosh);
M(tanh);
M(asinh);
M(acosh);
M(atanh);

// multi-argument functions
#define M2(x) xns_object *xns_prim_ ## x (struct xns_vm *vm, xns_obj env, xns_obj args){\
    if (xns_len(args) != 2)return vm->T;\
    R(args); R(env);\
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));R(arg1);\
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));\
    if(xns_nil(arg1)||xns_nil(arg2)) return vm->T;\
    U(args); U(env); U(arg1);\
    double res = x (arg1->dval, arg2->dval);\
    arg1 = xns_make_double(vm, res);\
    return arg1; }
M2(pow);
M2(hypot);
M2(jn);
M2(yn);