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

/// Implementation is almost straight out of J. McCarthy et al 1961

xns_object *eval(xns_obj obj, xns_obj env){
    if(!obj){
        return env->vm->NIL;
    }
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
            return obj;
        //////// critical case: CONS
        case XNS_CONS:
            // This differs from the paper in that the primitives are not implemented inline
            // and to allow for this, the arguements ARE NOT EVALUATED!
            printf("APPLYING FCN %s\n", obj->car->symname);
            xns_gc_register(obj->vm, &obj);
            xns_gc_register(obj->vm, &env);
            xns_obj val = eval(obj->car, env);
            xns_gc_register(obj->vm, &val);
            xns_print_object(val);
            xns_obj ret = apply(val, env, obj->cdr);
            xns_gc_unregister(obj->vm, &obj);
            xns_gc_unregister(obj->vm, &env);
            xns_gc_unregister(obj->vm, &val);
            return ret;
        default:
            // ??? -- should this error?
            return obj;
    }
    return NULL;
}
xns_object *evlis(xns_obj obj, xns_obj env){
    if(obj->type != XNS_CONS) return obj;
    xns_gc_register(obj->vm, &obj);
    xns_gc_register(obj->vm, &env);
    xns_obj ret = obj->vm->NIL;
    xns_gc_register(obj->vm, &ret);
    ret = xns_cons(obj->vm, eval(xns_car(obj), env), evlis(xns_cdr(obj), env));
    xns_gc_unregister(obj->vm, &obj);
    xns_gc_unregister(obj->vm, &env);
    xns_gc_unregister(obj->vm, &ret);
    return ret;
}
// this is a seperate function so that xns_prim_macroexpand doesn't have to duplicate apply
xns_object *macroexpand(xns_obj fn, xns_obj env, xns_obj args){
    xns_gc_register(fn->vm, &fn);
    xns_gc_register(fn->vm, &env);
    xns_gc_register(fn->vm, &args);
    xns_obj newenv = xns_make_env(fn->vm, fn->env);
    xns_gc_register(fn->vm, &newenv);
    newenv->vars = xns_pair(fn->args, args);
    xns_obj expansion = eval(fn->body, newenv);
    xns_obj ret = eval(expansion, env);
    xns_gc_unregister(fn->vm, &fn);
    xns_gc_unregister(fn->vm, &env);
    xns_gc_unregister(fn->vm, &args);
    xns_gc_unregister(fn->vm, &newenv);
    return ret;
}

xns_object *apply(xns_obj fn, xns_obj env, xns_obj args){
    if (fn->type == XNS_FUNCTION) {
        xns_gc_register(fn->vm, &fn);
        xns_gc_register(fn->vm, &env);
        xns_gc_register(fn->vm, &args);
        xns_obj newenv = xns_make_env(fn->vm, fn->env);
        xns_gc_register(fn->vm, &newenv);
        newenv->vars = xns_pair(fn->args, evlis(args, env));
        xns_obj ret = eval(fn->body, newenv);
        xns_gc_unregister(fn->vm, &fn);
        xns_gc_unregister(fn->vm, &env);
        xns_gc_unregister(fn->vm, &args);
        xns_gc_unregister(fn->vm, &newenv);
        return ret;
    } else if (fn->type == XNS_MACRO) {
        // hopefully this works ...
        xns_gc_register(fn->vm, &fn);
        xns_gc_register(fn->vm, &args);
        xns_obj ret = eval(macroexpand(fn, env, args), env);
        xns_gc_unregister(fn->vm, &fn);
        xns_gc_unregister(fn->vm, &args);
        return ret;
    } else if (fn->type == XNS_PRIMITIVE){
        printf("PRIMITIVE\n");
        return fn->primitive(fn->vm, env, args);
    } else if (fn->type == XNS_FOREIGN_FUNC){
        return fn->foreign_fcn(args, env);
    } else if (fn->type == XNS_HANDLE) {
        return apply(fn->ptr, env, args);
    } else {
        // ERROR
        return fn;
    }
}