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
#include "xns_lisp.h"

static void rp(struct xns_vm *vm, char *name, xns_primitive op){
    xns_object *obj = xns_make_primitive(vm, op);
    xns_gc_register(vm, &obj);
    xns_object *n = xns_intern(vm, name);
    xns_set(vm->env, n, obj);
    xns_gc_unregister(vm, &obj);
}

// Internal function -- called on VM startup
void xns_register_primops(struct xns_vm *vm){
    #include "primops.inc"
}
#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)
//symbols and gensyms
xns_object *xns_prim_exit(struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    // exit
    exit(0);
}
xns_object *xns_prim_eq     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
   if(xns_len(args) != 2){
       // TODO ERROR
       return vm->NIL;
   }
   R(env);
   struct xns_object *obj = eval(args->car, env);
   R(obj);
   struct xns_object *other=eval(args->cdr->car, env);
   if (xns_eq(obj, other)) {
       U(obj);
       U(env);
       return vm->T;
   }
   U(obj);
   U(env);
   return vm->NIL;
}
xns_object *xns_prim_null   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    struct xns_object *res = eval(args->car, env);
    if (xns_nil(res)) {
        res = vm->T;
    } else {
        res = vm->NIL;
    }
    return res;
}
xns_object *xns_prim_quote    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    return args->car;
}
// environment
xns_object *xns_prim_set    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    xns_object *val = eval(args->cdr->car, env);
    xns_object *sym = args->car;
    R(sym);
    R(val);
    xns_set(env, sym, val);
    U(val);
    U(sym);
    U(args);
    U(env);
    return val;
}

xns_object *xns_prim_assoc  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    xns_object *key = eval(args->car, env);
    R(key);
    xns_object *list = eval(args->cdr->car, env);
    xns_object *rtn = vm->NIL;
    while(!xns_nil(list)){
        if (xns_eq(key, list->car->car)){
            rtn = list->car;
            break;
        }
        list = list->cdr;
    }
    U(key);
    U(args);
    U(env);
    return rtn;
}
xns_object *xns_prim_eval   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    return eval(args, env);
}
xns_object *xns_prim_apply   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    xns_object *fn = eval(args->car, env);
    xns_object *ret = apply(fn, env, args->cdr);
    U(env);
    U(args);
    return ret;
}
// control
xns_object *xns_prim_let    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    xns_object *newenv = xns_make_env(vm, env);
    R(newenv);
    xns_object *vpairs = args->car;
    R(vpairs);
    xns_object *code = args->cdr;
    R(code);
    // parse vpairs
    while(!xns_nil(vpairs)){
        xns_object *o = vpairs->car;
        R(o);
        xns_object *val = eval(o->cdr->car, env);
        xns_set(newenv, o->car, val);
        U(o);
        vpairs = vpairs->cdr;
    }
    // run code
    xns_object *ret = vm->NIL;
    R(ret);
    while(!xns_nil(code)){
        ret = eval(code->car, newenv);
        code = code->cdr;
    }
    U(ret);
    U(code);
    U(vpairs);
    U(newenv);
    U(args);
    U(env);
    return ret;
}
xns_object *xns_prim_cond   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(args);
    R(env);
    while(!xns_nil(args)){
        xns_object *a = eval(args->car->car, env);
        if(!xns_nil(a)){
            U(env);
            U(args);
            return eval(args->car->cdr->car, env);
        }
        args = args->cdr;
    }
    U(env);
    U(args);
    return vm->NIL;
}

xns_object *xns_prim_lambda (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    return xns_make_function(vm, args->car, args->cdr, env);
}

xns_object *xns_prim_labels  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    xns_object *newenv = xns_make_env(vm, env);
    R(newenv);
    xns_object *vpairs = args->car;
    R(vpairs);
    xns_object *code = args->cdr;
    R(code);
    // parse vpairs
    while(!xns_nil(vpairs)){
        xns_object *o = vpairs->car;
        R(o);
        xns_object *val = eval(o->cdr->car, newenv);
        xns_set(newenv, o->car, val);
        U(o);
        vpairs = vpairs->cdr;
    }
    // run code
    xns_object *ret = vm->NIL;
    R(ret);
    while(!xns_nil(code)){
        ret = eval(code->car, newenv);
        code = code->cdr;
    }
    U(ret);
    U(code);
    U(vpairs);
    U(newenv);
    U(args);
    U(env);
    return ret;
}
// Cons Cells
xns_object *xns_prim_car    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if(!xns_nil(args->cdr)){
        // TODO ERROR
    }
    return eval(args->car, env)->car;
}
xns_object *xns_prim_cdr    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if(!xns_nil(args->cdr)){
        // TODO ERROR
    }
    return eval(args->car, env)->car;
}
xns_object *xns_prim_cons   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if(xns_len(args) != 2){
        // TODO ERROR
    }
    R(env);
    R(args);
    xns_object *first = eval(args->car, env);
    R(first);
    xns_object *second = eval(args->cdr->car, env);
    return xns_cons(vm, first, second);
}
xns_object *xns_prim_pair   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if (xns_len(args) != 2){
        // TODO ERROR
        return vm->NIL;
    }
    R(env);
    R(args);
    struct xns_object *first = eval(args->car, env);
    R(first);
    struct xns_object *second = eval(args->cdr->car, env);
    R(second);
    xns_object *p = xns_pair(first, second);
    U(env);
    U(args);
    U(first);
    U(second);
    return p;
}
xns_object *xns_prim_append (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if (xns_len(args) != 2){
        // TODO ERROR
        return vm->NIL;
    }
    R(env);
    R(args);
    struct xns_object *first = eval(args->car, env);
    R(first);
    struct xns_object *second = eval(args->cdr->car, env);
    R(second);
    xns_object *p = xns_append(first, second);
    U(env);
    U(args);
    U(first);
    U(second);
    return p;
}
//Predicates
xns_object *xns_prim_atom   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if (!xns_nil(args->cdr)){
        // TODO ERROR
        return vm->NIL;
    }
    struct xns_object *o = eval(args->car, env);
    while(o->type == XNS_HANDLE){
        o = o->ptr;
    }
    if (o->type != XNS_CONS){
        return vm->T;
    }
    return vm->NIL;
}
xns_object *xns_prim_and    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    struct xns_object *o = vm->NIL;
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
xns_object *xns_prim_or     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    R(env);
    R(args);
    struct xns_object *o = vm->NIL;
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
xns_object *xns_prim_not    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args){
    if (xns_len(args) != 1){
        // TODO ERROR
        return vm->NIL;
    }
    struct xns_object *expr = eval(args->car, env);
    if(xns_nil(expr)){
        return vm->T;
    }
    return vm->NIL;
}

//numbers
/// arith ops
xns_object *xns_prim_plus   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_minus  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_mult   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_divide (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
/// basic ops
xns_object *xns_prim_isnan  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_abs    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_mod    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_sqrt   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cbrt   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_hypot  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_expt   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_pow   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_log    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
/// advanced ops
xns_object *xns_prim_erf    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_erfc   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_gamma  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_lgamma (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_j0     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_j1     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_jn     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_y0     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_y1     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_yn     (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_exp1m   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
/// rounding
xns_object *xns_prim_floor  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_ceil   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_round   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);

/// trig ops
xns_object *xns_prim_sin    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cos    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_tan    (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_asin   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_acos   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_atan   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
/// hyperbolic trig
xns_object *xns_prim_sinh   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_cosh   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_tanh   (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_asinh  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_acosh  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);
xns_object *xns_prim_atanh  (struct xns_vm *vm, struct xns_object *env, struct xns_object *args);