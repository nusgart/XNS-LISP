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