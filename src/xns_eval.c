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
// XNS-Lisp evaluator
#include "xns_lisp.h"
#include <sys/types.h>
#include <regex.h>

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)
/// Implementation is almost straight out of J. McCarthy et al 1961

xns_object *eval(xns_obj obj, xns_obj env){
    if(!obj){
        return env->vm->NIL;
    }
    struct xns_vm *vm = obj->vm;
    switch(obj->type){
        // handle --> eval handle
        case XNS_HANDLE:
            return eval(obj->ptr, env);
        // lookup symbol value
        case XNS_GENSYM:
        case XNS_SYMBOL:
            // check for self-evaluating
            if(obj->symname[0] == ':')
                return obj;
            return xns_assoc(env, obj);
        // self-evaluating
        case XNS_RATIONAL:
        case XNS_INTEGER:
        case XNS_DOUBLE:
        case XNS_FIXNUM:
        case XNS_STRING:
        case XNS_ARRAY:
            return obj;
        //////// critical case: CONS
        case XNS_CONS:
            // This differs from the paper in that the primitives are not implemented inline
            // and to allow for this, the arguements ARE NOT EVALUATED!
            ;char *desc = xns_to_string(obj);
            fprintf(vm->debug, "APPLYING FCN %s -- %s", obj->car->symname, desc);
            free(desc);
            R(obj); R(env);
            xns_obj val = eval(obj->car, env);
            R(val);
            desc = xns_to_string(val);
            fprintf(vm->debug, " value %s\n", desc);
            free(desc);
            xns_obj ret = apply(val, env, obj->cdr);
            U(obj); U(env); U(val);
            ptrdiff_t diff = (char*)ret - (char*)vm->heap.current_heap;
            // this assumes that the heap is smaller than PTRDIFF_MAX chars, which is enforced.
            if(diff < 0 || diff > (ptrdiff_t)vm->heap.size){
                vm->warning(vm, "xns_eval: object is out of bounds!", obj);
                fprintf(stderr, "xns_eval %p probable fail\n", ret);
                fflush(stderr);
                vm->error(vm, "xns_eval: object out of bounds", ret);
            }
            return ret;
        default:
            // ??? -- should this error?
            vm->error(vm, "Bad object type in eval", obj);
            return obj;
    }
    vm->error(vm, "Eval executing an unreachable segment", NULL);
    return NULL;
}
xns_object *evlis(xns_obj obj, xns_obj env){
    if(obj->type != XNS_CONS && !xns_nil(obj)) {
        // TODO either error or better warning
        char *desc = xns_to_string(obj);
        fprintf(stderr, "WARNING: evlis on non-cons object %u with type %s value %s\n", obj->object_id, xns_type_to_string(obj->type), desc);
        free(desc);
        return obj;
    }
    if(xns_nil(obj)){
        return obj;
    }
    struct xns_vm *vm = obj->vm;
    xns_obj ret = obj->vm->NIL;
    R(obj); R(env); R(ret);
    ret = xns_cons(obj->vm, eval(xns_car(obj), env), evlis(xns_cdr(obj), env));
    U(obj); U(env); U(ret);
    return ret;
}
// this is a seperate function so that xns_prim_macroexpand doesn't have to duplicate apply
xns_object *macroexpand(xns_obj fn, xns_obj env, xns_obj args){
    struct xns_vm *vm = fn->vm;
    R(fn); R(env); R(args);
    xns_obj newenv = xns_make_env(fn->vm, fn->env);
    R(newenv);
    #ifndef NO_REST
    {
        xns_obj key = fn->args, val = args;
        R(key); R(val);
        for(; !xns_nil(key) && !xns_nil(val); key = key->cdr, val = val->cdr){
            if(key->type != XNS_CONS){
                //TODO ERROR
                fprintf(stderr, "INVALID FUNCTION SIGNATURE!!!!!!\n");
                abort();
            }
            if (val->type != XNS_CONS){
                fprintf(stderr, "INVALID FUNCTION SIGNATURE!!!!!!\n");
                abort();
            }
            if(xns_eq(key->car, fn->vm->rest)){
                xns_set(newenv, key->cdr->car, val);
                break;
            }
            xns_set(newenv, key->car, val->car);
        }
        U(key); U(val);
    }
    #else
    newenv->vars = xns_pair(fn->args, args);
    #endif
    //xns_obj expansion = eval(fn->body, newenv);
    xns_obj ip = fn->body, ret = vm->NIL;
        R(ip); R(ret);
        while(!xns_nil(ip)){
            ret = eval(ip->car, newenv);
            ip = ip->cdr;
        }
        U(ip); U(ret);
    //xns_obj ret = eval(expansion, env);
    U(fn); U(env); U(args); U(newenv);
    return ret;
}

xns_object *apply(xns_obj fn, xns_obj env, xns_obj args){
    if (!fn) {
        fprintf(stderr, "Apply called with NULL function object!\n");
        abort();
    }
    struct xns_vm *vm = fn->vm;
    if (fn->type == XNS_FUNCTION) {
        R(fn); R(env); R(args);
        xns_obj newenv = xns_make_env(fn->vm, fn->env);
        xns_gc_register(fn->vm, &newenv);
        #ifndef NO_REST
        {
            xns_obj evarg = evlis(args, env);
            xns_obj key = fn->args, val = evarg;
            R(key); R(val);
            for(; !xns_nil(key) && !xns_nil(val); key = key->cdr, val = val->cdr){
                if(xns_eq(key->car, fn->vm->rest)){
                    xns_set(newenv, key->cdr->car, val);
                    break;
                }
                xns_set(newenv, key->car, val->car);
            }
            U(key); U(val);
        }
        #else
        newenv->vars = xns_pair(fn->args, evlis(args, env));
        #endif
        xns_obj ip = fn->body, ret = vm->NIL;
        R(ip); R(ret);
        while(!xns_nil(ip)){
            ret = eval(ip->car, newenv);
            ip = ip->cdr;
        }
        U(ip); U(ret);
        U(fn); U(env); U(args); U(newenv);
        return ret;
    } else if (fn->type == XNS_MACRO) {
        // hopefully this works ...
        xns_gc_register(fn->vm, &fn);
        xns_gc_register(fn->vm, &args);
        xns_obj expansion = macroexpand(fn, env, args);
        printf("EXPANSION ");xns_print_object(expansion);
        xns_obj ret = eval(expansion, env);
        xns_gc_unregister(fn->vm, &fn);
        xns_gc_unregister(fn->vm, &args);
        return ret;
    } else if (fn->type == XNS_PRIMITIVE){
        return fn->primitive(fn->vm, env, args);
    } else if (fn->type == XNS_FOREIGN_FUNC){
        return fn->foreign_fcn(args, env);
    } else if (fn->type == XNS_HANDLE) {
        return apply(fn->ptr, env, args);
    } else {
        // ERROR
        unsigned int id = fn->object_id;
        char *type = xns_type_to_string(fn->type);
        char *desc = xns_to_string(fn);
        fprintf(stderr, "Apply called on non-function object (id %u type %s value %s!\n", id, type, desc);
        free(desc);
        return fn;
    }
}