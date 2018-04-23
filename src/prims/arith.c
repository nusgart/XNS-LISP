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
// XNS-Lisp arithmetic operator implementation
#include "xns_lisp.h"

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)

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
            case XNS_STRING:
                type = XNS_STRING;// special case of +
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
        case XNS_STRING:;
            char *out = strdup(""), *tmp = NULL;
            for(xns_obj a = ns; !xns_nil(a); a = a->cdr){
                char *desc;
                if(a->car->type != XNS_STRING) desc = xns_to_string(a->car);
                else desc = strdup(a->car->string);
                asprintf(&tmp, "%s%s", out, desc);
                free(desc); free(out);
                out = tmp;
            }
            ret = xns_make_string(vm, out);
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
                if(aaa){
                    lsum = a->car->fixnum;
                } else {
                    if (a->car->fixnum == 0){
                        vm->error(vm, "Integer division by Zero!", args);
                        goto error;
                    }
                    lsum /= a->car->fixnum;;
                }
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
    error:
    U(ret);
    U(ns);
    U(args);
    U(env);
    return ret;
}
// greater/lesser
xns_object *xns_prim_greater  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 1){
        return vm->NIL;
    }
    R(args); R(env);
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));
    R(arg1);
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));
    if(xns_nil(arg1) || xns_nil(arg2)){
        return vm->T;
    }
    U(env); U(args); U(arg1);
    if(arg1->dval > arg2->dval){
        return vm->T;
    } else { 
        return vm->NIL;
    }
}

xns_object *xns_prim_lesser  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        return vm->NIL;
    }
    R(args); R(env);
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));
    R(arg1);
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));
    if(xns_nil(arg1) || xns_nil(arg2)){
        return vm->T;
    }
    U(env); U(args); U(arg1);
    if(arg1->dval < arg2->dval){
        return vm->T;
    } else { 
        return vm->NIL;
    }
}

xns_object *xns_prim_equal  (struct xns_vm *vm, xns_obj env, xns_obj args){
    if (xns_len(args) != 2){
        return vm->T;
    }
    R(args); R(env);
    xns_obj arg1 = xns_to_double(vm, eval(args->car, env));
    R(arg1);
    xns_obj arg2 = xns_to_double(vm, eval(args->cdr->car, env));
    if(xns_nil(arg1) || xns_nil(arg2)){
        return vm->T;
    }
    U(env); U(args); U(arg1);
    if(arg1->dval == arg2->dval){
        return vm->T;
    } else { 
        return vm->NIL;
    }
}