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
    xns_set(vm->env, n, obj);
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
       return vm->NIL;
   }
   R(env);
   xns_obj obj = eval(args->car, env);
   R(obj);
   xns_obj other=eval(args->cdr->car, env);
   if (xns_eq(obj, other)) {
       U(obj);
       U(env);
       return vm->T;
   }
   U(obj);
   U(env);
   return vm->NIL;
}
xns_object *xns_prim_null   (struct xns_vm *vm, xns_obj env, xns_obj args){
    xns_obj res = eval(args->car, env);
    if (xns_nil(res)) {
        res = vm->T;
    } else {
        res = vm->NIL;
    }
    return res;
}
xns_object *xns_prim_quote    (struct xns_vm *vm, xns_obj env, xns_obj args){
    (void) vm; (void) env;
    return args->car;
}
// environment
xns_object *xns_prim_set    (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj val = eval(args->cdr->car, env);
    xns_obj sym = args->car;
    R(sym);
    R(val);
    xns_set(env, sym, val);
    U(val);
    U(sym);
    U(args);
    U(env);
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
    R(env);
    R(args);
    xns_obj key = eval(args->car, env);
    R(key);
    xns_obj list = eval(args->cdr->car, env);
    xns_obj rtn = vm->NIL;
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
        xns_set(newenv, o->car, val);
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

xns_object *xns_prim_lambda (struct xns_vm *vm, xns_obj env, xns_obj args){
    return xns_make_function(vm, args->car, args->cdr, env);
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
        xns_set(newenv, o->car, val);
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
        // TODO ERROR
    }
    (void) vm;
    return eval(args->car, env)->car;
}
xns_object *xns_prim_cdr    (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(!xns_nil(args->cdr)){
        // TODO ERROR
    }
    (void) vm;
    return eval(args->car, env)->car;
}
xns_object *xns_prim_cons   (struct xns_vm *vm, xns_obj env, xns_obj args){
    if(xns_len(args) != 2){
        // TODO ERROR
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
        // TODO ERROR
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
xns_object *xns_prim_append (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        // TODO ERROR
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
    if (!xns_nil(args->cdr)){
        // TODO ERROR
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
        // TODO ERROR
        return vm->NIL;
    }
    xns_obj expr = eval(args->car, env);
    if(xns_nil(expr)){
        return vm->T;
    }
    return vm->NIL;
}

//numbers
/// arith ops
xns_object *xns_prim_plus   (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj ns = evlis(args, env);
    R(ns);
    enum xns_type type = XNS_FIXNUM;
    for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
        switch(a->car->type){
            case XNS_FIXNUM: break;
            case XNS_DOUBLE: 
                if(type == XNS_FIXNUM)
                    type = XNS_DOUBLE;
                else if (type == XNS_INTEGER){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_INTEGER:
                if(type == XNS_FIXNUM)
                    type = XNS_INTEGER;
                else if (type == XNS_DOUBLE){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_RATIONAL:
                type = XNS_RATIONAL;
                break;
            default:
                // TODO ERROR
                break;
        }
    }
    xns_obj ret = vm->NIL;
    R(ret);
    switch(type){
        case XNS_FIXNUM:;
            long lsum = 0;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                lsum += a->car->fixnum;
            }
            ret = xns_make_fixnum(vm, lsum);
            break;
        case XNS_DOUBLE:;
            double dsum = 0.0;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                dsum += xns_to_double(vm, a->car)->dval;
            }
            ret = xns_make_double(vm, dsum);
            break;
        case XNS_RATIONAL:
        case XNS_INTEGER:
            // TODO IMPLEMENT
            break;
        default: break;//TODO ERROR
    }
    U(ret);
    U(ns);
    U(args);
    U(env);
    return ret;
}
xns_object *xns_prim_minus  (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj ns = evlis(args, env);
    R(ns);
    enum xns_type type = XNS_FIXNUM;
    for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
        switch(a->car->type){
            case XNS_FIXNUM: break;
            case XNS_DOUBLE: 
                if(type == XNS_FIXNUM)
                    type = XNS_DOUBLE;
                else if (type == XNS_INTEGER){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_INTEGER:
                if(type == XNS_FIXNUM)
                    type = XNS_INTEGER;
                else if (type == XNS_DOUBLE){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_RATIONAL:
                type = XNS_RATIONAL;
                break;
            default:
                // TODO ERROR
                break;
        }
    }
    xns_obj ret = vm->NIL;
    R(ret);
    switch(type){
        case XNS_FIXNUM:;
            long lsum = 0;
            long ls = 1;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                lsum += ls * a->car->fixnum;
                ls = -1;
            }
            ret = xns_make_fixnum(vm, lsum);
            break;
        case XNS_DOUBLE:;
            double dsum = 0.0;
            double ds = 1.0;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                dsum += ds * xns_to_double(vm, a->car)->dval;
                ds = -1.0;
            }
            ret = xns_make_double(vm, dsum);
            break;
        case XNS_RATIONAL:
        case XNS_INTEGER:
            // TODO IMPLEMENT
            break;
        default: break;//TODO ERROR
    }
    U(ret);
    U(ns);
    U(args);
    U(env);
    return ret;
}
xns_object *xns_prim_mult   (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj ns = evlis(args, env);
    R(ns);
    enum xns_type type = XNS_FIXNUM;
    for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
        switch(a->car->type){
            case XNS_FIXNUM: break;
            case XNS_DOUBLE: 
                if(type == XNS_FIXNUM)
                    type = XNS_DOUBLE;
                else if (type == XNS_INTEGER){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_INTEGER:
                if(type == XNS_FIXNUM)
                    type = XNS_INTEGER;
                else if (type == XNS_DOUBLE){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_RATIONAL:
                type = XNS_RATIONAL;
                break;
            default:
                // TODO ERROR
                break;
        }
    }
    xns_obj ret = vm->NIL;
    R(ret);
    switch(type){
        case XNS_FIXNUM:;
            long lsum = 1;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                lsum *= a->car->fixnum;
            }
            ret = xns_make_fixnum(vm, lsum);
            break;
        case XNS_DOUBLE:;
            double dsum = 1.0;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                dsum *= xns_to_double(vm, a->car)->dval;
            }
            ret = xns_make_double(vm, dsum);
            break;
        case XNS_RATIONAL:
        case XNS_INTEGER:
            // TODO IMPLEMENT
            break;
        default: break;//TODO ERROR
    }
    U(ret);
    U(ns);
    U(args);
    U(env);
    return ret;
}
xns_object *xns_prim_divide (struct xns_vm *vm, xns_obj env, xns_obj args){
    R(env);
    R(args);
    xns_obj ns = evlis(args, env);
    R(ns);
    enum xns_type type = XNS_FIXNUM;
    for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
        switch(a->car->type){
            case XNS_FIXNUM: break;
            case XNS_DOUBLE: 
                if(type == XNS_FIXNUM)
                    type = XNS_DOUBLE;
                else if (type == XNS_INTEGER){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_INTEGER:
                if(type == XNS_FIXNUM)
                    type = XNS_INTEGER;
                else if (type == XNS_DOUBLE){
                    type = XNS_RATIONAL;
                }
                break;
            case XNS_RATIONAL:
                type = XNS_RATIONAL;
                break;
            default:
                // TODO ERROR
                break;
        }
    }
    xns_obj ret = vm->NIL;
    R(ret);
    switch(type){
        case XNS_FIXNUM:;
            long lsum = 0;
            bool aaa = true;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                lsum = (aaa) ? a->car->fixnum: lsum / a->car->fixnum;
                aaa = false;
            }
            ret = xns_make_fixnum(vm, lsum);
            break;
        case XNS_DOUBLE:;
            double dsum = 1.0;
            bool dfirst = true;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                dsum = dfirst? xns_to_double(vm, a->car)->dval: dsum / xns_to_double(vm, a->car)->dval;
                dfirst = false;
            }
            ret = xns_make_double(vm, dsum);
            break;
        case XNS_RATIONAL:
        case XNS_INTEGER:
            // TODO IMPLEMENT
            break;
        default: break;//TODO ERROR
    }
    U(ret);
    U(ns);
    U(args);
    U(env);
    return ret;
}