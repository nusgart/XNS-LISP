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

static void rp(struct xns_vm *vm, char *name, xns_primitive op){
    xns_obj obj = xns_make_primitive(vm, op);
    xns_gc_register(vm, &obj);
    xns_obj n = xns_intern(vm, name);
    xns_bind(vm->env, n, obj);
    xns_gc_unregister(vm, &obj);
}

// Internal function -- called on VM startup
void xns_register_primops(struct xns_vm *vm){
    #include "primops.inc"
}

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)


xns_object *xns_prim_exit(struct xns_vm *vm, xns_obj env, xns_obj args){
    (void)vm; (void)env; (void)args;
    exit(0);
}
//symbols and gensyms
xns_object *xns_prim_eq     (struct xns_vm *vm, xns_obj env, xns_obj args){
   if(xns_len(args) != 2){
       // TODO ERROR
       vm->error(vm, "eq called with too many arguments", args);
       return vm->NIL;
   }
   R(env);
   xns_obj obj = eval(args->car, env);
   R(obj);
   xns_obj other=eval(args->cdr->car, env);
   if (xns_eq(obj, other)) {
       U(obj); U(env);
       return vm->T;
   }
   U(obj); U(env);
   return vm->NIL;
}
xns_object *xns_prim_null   (struct xns_vm *vm, xns_obj env, xns_obj args){
    xns_obj res = eval(args->car, env);
    return xns_nil(res) ? vm->T: vm->NIL;
}
xns_object *xns_prim_quote    (struct xns_vm *vm, xns_obj env, xns_obj args){
    (void) vm; (void) env;
    return args->car;
}

xns_object *xns_prim_gensym (struct xns_vm *vm, xns_obj env, xns_obj args) {
    (void) env; (void) args;
    return xns_gensym(vm);
}

xns_object *xns_prim_intern (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(xns_len(args) != 1){
        vm->error(vm, "intern called with an incorrect number of arguments", args);
        return vm->NIL;
    }
    xns_obj str = eval(args->car, env);
    return xns_intern(vm, str->string);
}

// environment
xns_object *xns_prim_set    (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env); R(args);
    xns_obj val = eval(args->cdr->car, env);
    xns_obj sym = args->car;
    R(sym); R(val);
    xns_set(env, sym, val);
    U(val); U(sym); U(args); U(env);
    return val;
}

xns_object *xns_prim_define (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(args);
    xns_obj val = eval(args->cdr->car, env);
    xns_obj sym = args->car;
    R(val);
    xns_set(vm->env, sym, val);
    U(val);
    U(args);
    return val;
}

xns_object *xns_prim_assoc  (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env); R(args);
    xns_obj key = eval(args->car, env);
    R(key);
    xns_obj list = eval(args->cdr->car, env);
    xns_obj rtn = vm->NIL;
    if (list->type != XNS_CONS) {
        goto error;
    }
    while(!xns_nil(list)){
        if (xns_eq(key, list->car->car)) {
            rtn = list->car;
            break;
        }
        list = list->cdr;
    }
    error:
    U(key); U(args); U(env);
    return rtn;
}
xns_object *xns_prim_eval   (struct xns_vm *vm, xns_obj env, xns_obj args){
    (void)vm;
    return eval(args, env);
}
xns_object *xns_prim_apply   (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj fn = eval(args->car, env);
    xns_obj ret = apply(fn, env, args->cdr);
    U(env);
    U(args);
    return ret;
}
// control
xns_object *xns_prim_let    (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj newenv = xns_make_env(vm, env);
    R(newenv);
    xns_obj vpairs = args->car;
    R(vpairs);
    xns_obj code = args->cdr;
    R(code);
    // parse vpairs
    while(!xns_nil(vpairs)){
        xns_obj o = vpairs->car;
        R(o);
        xns_obj val = eval(o->cdr->car, env);
        xns_bind(newenv, o->car, val);
        U(o);
        vpairs = vpairs->cdr;
    }
    // run code
    xns_obj ret = vm->NIL;
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
xns_object *xns_prim_cond   (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(args);
    R(env);
    while(!xns_nil(args)){
        xns_obj a = eval(args->car->car, env);
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

xns_object *xns_prim_while (struct xns_vm *vm, xns_obj env, xns_obj args){
    xns_obj pred = args->car;
    xns_obj code = args->cdr;
    R(pred); R(code);
    bool cont = (eval(pred, env) != vm->NIL);
    while(cont){
        xns_obj cc = code;
        R(cc);
        while(!xns_nil(cc)){
            eval(cc->car, env);
            cc = cc->cdr;
        }
        U(cc);
        cont = (eval(pred, env) != vm->NIL);
    }
    U(pred); U(code);
    return vm->NIL;
}

xns_object *xns_prim_lambda (struct xns_vm *vm, xns_obj env, xns_obj args){
    return xns_make_function(vm, args->car, args->cdr, env);
}

xns_object *xns_prim_mlambda(struct xns_vm *vm, xns_obj env, xns_obj args){
    return xns_make_macro(vm, args->car, args->cdr, env);
}

xns_object *xns_prim_labels  (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj newenv = xns_make_env(vm, env);
    R(newenv);
    xns_obj vpairs = args->car;
    R(vpairs);
    xns_obj code = args->cdr;
    R(code);
    // parse vpairs
    while(!xns_nil(vpairs)){
        xns_obj o = vpairs->car;
        R(o);
        xns_obj val = eval(o->cdr->car, newenv);
        xns_bind(newenv, o->car, val);
        U(o);
        vpairs = vpairs->cdr;
    }
    // run code
    xns_obj ret = vm->NIL;
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
xns_object *xns_prim_car    (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(!xns_nil(args->cdr)){
        vm->error(vm, "car called with too many arguments", args);
    }
    (void) vm;
    return xns_car(eval(args->car, env));
}
xns_object *xns_prim_cdr    (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(!xns_nil(args->cdr)){
        vm->error(vm, "cdr called with too many arguments", args);
    }
    (void) vm;
    return xns_cdr(eval(args->car, env));
}
xns_object *xns_prim_cons   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(xns_len(args) != 2){
        vm->error(vm, "cons called with an incorrect number of arguments", args);
    }
    R(env);
    R(args);
    xns_obj first = eval(args->car, env);
    R(first);
    xns_obj second = eval(args->cdr->car, env);
    return xns_cons(vm, first, second);
}
xns_object *xns_prim_pair   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        vm->error(vm, "pair called with an incorrect number of arguments", args);
        return vm->NIL;
    }
    R(env);
    R(args);
    xns_obj first = eval(args->car, env);
    R(first);
    xns_obj second = eval(args->cdr->car, env);
    R(second);
    xns_obj p = xns_pair(first, second);
    U(env);
    U(args);
    U(first);
    U(second);
    return p;
}

xns_object *xns_prim_setcar(struct xns_vm *vm, xns_obj env, xns_obj args){
    if(xns_len(args) != 2){
        vm->error(vm, "setcdr called with the wrong number of arguments", args);
    }
    R(env); R(args);
    xns_obj val = eval(args->cdr->car, env);
    R(val);
    xns_obj cons = eval(args->car, env);
    R(cons);
    if (cons->type == XNS_CONS) {
        cons->car = val;
    } else {
        vm->error(vm, "setcar called with wrong argument type", cons);
        U(val); U(cons); U(args); U(env);
        return vm->NIL;
    }
    U(val); U(cons); U(args); U(env);
    return cons;
}

xns_object *xns_prim_setcdr(struct xns_vm *vm, xns_obj env, xns_obj args){
    if(xns_len(args) != 2){
        vm->error(vm, "setcdr called with the wrong number of arguments", args);
    }
    R(env); R(args);
    xns_obj val = eval(args->cdr->car, env);
    R(val);
    xns_obj cons = eval(args->car, env);
    R(cons);
    if (cons->type == XNS_CONS) {
        cons->cdr = val;
    } else {
        vm->error(vm, "setcdr called with wrong argument type", cons);
        U(val); U(cons); U(args); U(env);
        return vm->NIL;
    }
    U(val); U(cons); U(args); U(env);
    return cons;
}

xns_object *xns_prim_append (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        vm->error(vm, "append called with an incorrect number of arguments", args);
        return vm->NIL;
    }
    R(env);
    R(args);
    xns_obj first = eval(args->car, env);
    R(first);
    xns_obj second = eval(args->cdr->car, env);
    R(second);
    xns_obj p = xns_append(first, second);
    U(env);
    U(args);
    U(first);
    U(second);
    return p;
}
//Predicates
xns_object *xns_prim_atom   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        vm->error(vm, "atom called with an incorrect number of arguments", args);
        return vm->NIL;
    }
    xns_obj o = eval(args->car, env);
    while(o->type == XNS_HANDLE){
        o = o->ptr;
    }
    if (o->type != XNS_CONS){
        return o;
    }
    return vm->NIL;
}

xns_object *xns_prim_macroexpand (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(args); R(env);
    xns_obj macro = eval(args->car, env);
    if (macro->type != XNS_MACRO){
        vm->error(vm, "macroexpand called on non-macro!", args);
        return vm->NIL;
    }
    xns_obj ret = macroexpand(macro, env, args->cdr);
    U(args); U(env);
    return ret;
}

xns_object *xns_prim_progn  (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    xns_obj ip = args, ret = vm->NIL;
    R(ip); R(ret);
    while(!xns_nil(ip)){
        ret = eval(ip->car, env);
        ip = ip->cdr;
    }
    U(ip); U(ret); U(env);
    return ret;
}
extern xns_object *xns_numequal(struct xns_vm *vm, xns_obj env, xns_obj arg1, xns_obj arg2);

xns_object *xns_equal(struct xns_vm *vm, xns_obj env, xns_obj arg1, xns_obj arg2){
    // check for pointer equality or both being nil but one being NULL
    if (arg1 == arg2 || (xns_nil(arg1) && xns_nil(arg2))) {
        return vm->T;
    }
    if (xns_nil(arg1)) {
        return vm->NIL;
    }
    if (xns_nil(arg2)) {
        return vm->NIL;
    }
    // numeric types are handled elsewhere
    if (arg1->type == XNS_FIXNUM || arg1->type == XNS_DOUBLE || arg1->type == XNS_RATIONAL || arg1->type == XNS_INTEGER){
        return xns_numequal(vm, env, arg1, arg2);
    }
    // check for handles or moved values and follow the pointer to the direct value
    if (arg1->type == XNS_HANDLE || arg1->type == XNS_MOVED){
        return xns_equal(vm, env, arg1->ptr, arg2);
    }
    if (arg2->type == XNS_HANDLE || arg1->type == XNS_MOVED){
        return xns_equal(vm, env, arg1, arg2->ptr);
    }
    // if arg1 and arg2 are direct and non-numeric, then they must be the same type to be equal.
    if (arg1->type != arg2->type){
        return vm->NIL;
    }
    switch(arg1->type){
        case XNS_FUNCTION:
        case XNS_MACRO:
        case XNS_ENV:
        case XNS_PRIMITIVE:
            return vm->NIL;
        default:
            vm->error(vm, "Argument 1 to equal has unknown type", arg1);
            return vm->NIL;
        case XNS_GENSYM:
        case XNS_SYMBOL:
            return arg2->symid == arg1->symid? vm->T:vm->NIL;
        case XNS_FOREIGN_FUNC:
            return arg1->foreign_fcn == arg2->foreign_fcn? vm->T:vm->NIL; 
        case XNS_FOREIGN_PTR:
            return arg1->foreign_pointer == arg2->foreign_pointer?vm->T:vm->NIL;
        // TODO implement checks for self-containing arrays or lists.  This will likely require sets to be implemented.
        case XNS_MAP:
            return vm->NIL; // not implemented
        case XNS_ARRAY:
            if (arg1->len != arg2->len){
                return vm->NIL;
            }
            for(size_t idx = 0; idx < arg1->len; idx++){
                xns_object *pred = xns_equal(vm, env, arg1->array[idx], arg2->array[idx]);
                if (xns_nil(pred)){
                    return vm->NIL;
                }
            }
            return vm->T;
        case XNS_CONS:
            return xns_equal(vm, env, arg1->cdr, arg2->cdr);
        case XNS_STRING:
            if (arg1->len == arg2->len && !strncmp(arg1->string, arg2->string, arg1->len)){
                return vm->T;
            } else {
                return vm->NIL;
            }
    }
    return vm->NIL;
}

xns_object *xns_prim_equal(struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        return vm->T;
    }
    R(args); R(env);
    xns_obj arg1 = eval(args->car, env);
    R(arg1);
    xns_obj arg2 = eval(args->cdr->car, env);
    return xns_equal(vm, env, arg1, arg2);
}