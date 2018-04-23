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
#include "xns_lisp.h"

////symbols and gensyms

/**
 * xns_intern interns a symbol with the given name: that is, it either finds the existing symbol with
 * said name, or creates a new symbol with the given name.
 */
xns_object *xns_intern(xns_vm *vm, const char *name){
    xns_obj p = vm->symbols;
    while(p && p != vm->NIL){
        if(!strcmp(p->car->symname, name)){
            return p->car;
        }
        p = p->cdr;
    }
    size_t size = strlen(name) + 1 + sizeof(size_t);
    xns_obj obj = xns_alloc_object(vm, XNS_SYMBOL, size);
    obj->symid = vm->current_symbol++;
    strcpy(obj->symname, name);
    fprintf(vm->debug, "Added symbol %s with symid %lu objid %u\n", obj->symname, obj->symid, obj->object_id);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
// create a GENSYM -- a generated symbol that is guaranteed to have an unique value -- it will not be EQ to any other
//    symbol, even the SAME SYMBOL READ IN ANOTHER TIME: (EQ '#:G191 #:G191) evaluates to NIL!
xns_object *xns_gensym(xns_vm *vm){
    char gs[] = "#:G1234567890";
    xns_obj obj = xns_alloc_object(vm, XNS_GENSYM, sizeof(gs) + sizeof(size_t));
    char *gsym = obj->symname;
    obj->symid = vm->current_symbol++;
    snprintf(gsym, sizeof(gs), "#:G%lu", obj->symid);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
// are 
bool xns_eq(xns_obj a, xns_obj b){
    if(!a || !b) return !a && !b;
    if(a == b) return true;
    if(a->type != XNS_SYMBOL && a->type != XNS_GENSYM) return false;
    if(b->type != XNS_SYMBOL && b->type != XNS_GENSYM) return false;
    return a==b || (a->vm == b->vm && a->symid == b->symid);
}
// is obj nill? (Nill is the Lisp equivalent of NULL -- it is the empty list, false, and also a symbol)
bool xns_nil(xns_obj obj){
    return !obj || obj == obj->vm->NIL;
}
// environment
xns_object *xns_make_env(xns_vm *vm, xns_obj parent){
    xns_gc_register(vm, &parent);
    xns_obj obj = xns_alloc_object(vm, XNS_ENV, 2 * sizeof(xns_obj ));
    fprintf(vm->debug, "ENV Object %d has size %lu\n", obj->object_id, obj->size);
    obj->parent = parent? parent: vm->NIL;
    obj->vars = vm->NIL;
    xns_gc_unregister(vm, &parent);
    return obj;
}
/*
  An environment is implemented as an association list of symbols to values
 */
xns_object *xns_assoc(xns_obj env, xns_obj sym){
    xns_obj curr = env->vars;
    while(curr && curr != env->vm->NIL){
        assert(curr->type == XNS_CONS);
        if(xns_eq(curr->car->car, sym)){
            return curr->car->cdr;
        }
        curr = curr->cdr;
    }
    if(!env->parent || env->parent == env->vm->NIL){
        return env->vm->NIL;
    }
    return xns_assoc(env->parent, sym);
}
xns_object *xns_set(xns_obj env, xns_obj sym, xns_obj value){
    xns_gc_register(env->vm, &env);
    xns_gc_register(env->vm, &sym);
    xns_gc_register(env->vm, &value);
    xns_obj pair = xns_cons(env->vm, sym, value);
    xns_gc_register(env->vm, &pair);
    env->vars = xns_cons(env->vm, pair, env->vars);
    xns_gc_unregister(env->vm, &pair);
    xns_gc_unregister(env->vm, &env);
    xns_gc_unregister(env->vm, &sym);
    xns_gc_unregister(env->vm, &value);
    return value;
}
// Cons Cells
xns_object *xns_car(xns_obj obj){
    if(!obj) return NULL;
    if(obj->type != XNS_CONS){
        if(obj == obj->vm->NIL){
            return obj;
        }
        // TODO error
        obj->vm->error(obj->vm, "xns_car called on non-cons object", obj);
        return NULL;
    }
    return obj->car;
}
xns_object *xns_cdr(xns_obj obj){
    if(!obj) return NULL;
    if(obj->type != XNS_CONS){
        if(obj == obj->vm->NIL){
            return obj;
        }
        // TODO error
        obj->vm->error(obj->vm, "xns_cdr called on non-cons object", obj);
        return NULL;
    }
    return obj->cdr;
}
xns_object *xns_cons(xns_vm *vm, xns_obj car, xns_obj cdr){
    xns_gc_register(vm, &car);
    xns_gc_register(vm, &cdr);
    ptrdiff_t diff = (char*)car - (char*)vm->heap.current_heap;
    if(diff < 0 || diff > (ptrdiff_t)vm->heap.size){
        //
        vm->warning(vm, "xns_cons: object is out of bounds!", NULL);
        fprintf(stderr, "xns_cons %p probable fail\n", car);
        fflush(stderr);
        vm->error(vm, "xns_cons: object out of bounds", car);
    }
    xns_obj obj = xns_alloc_object(vm, XNS_CONS, 2 * sizeof(xns_obj ));
    if (obj > car && ((char*)obj - (char*)car) < (ptrdiff_t)car->size){
        vm->error(vm, "xns_cons: object overwritten!!!", car);
    }
    obj->car = car;
    obj->cdr = cdr;
    xns_gc_unregister(vm, &car);
    xns_gc_unregister(vm, &cdr);
    return obj;
}
xns_object *xns_nreverse(xns_obj list){
    xns_obj obj = list->vm->NIL;
    while(!xns_nil(list)){
        xns_obj hd = list;
        list = list->cdr;
        hd->cdr = obj;
        obj = hd;
    }
    return obj;
}

xns_object *xns_pair(xns_obj x, xns_obj y){
    xns_gc_register(x->vm, &x);
    xns_gc_register(x->vm, &y);
    xns_obj prs = x->vm->NIL;
    xns_gc_register(x->vm, &prs);
    while(!xns_nil(x) && !xns_nil(y)){
        prs = xns_cons(x->vm, xns_cons(x->vm, x->car,y->car), prs);
        x = x->cdr;
        y = y->cdr;
    }
    xns_gc_unregister(x->vm, &prs);
    xns_gc_unregister(x->vm, &y);
    xns_gc_unregister(x->vm, &x);
    return prs;
}

xns_object *xns_append(xns_obj x, xns_obj y){
    if(xns_nil(x)) return y;
    xns_gc_register(x->vm, &x);
    xns_gc_register(x->vm, &y);
    xns_obj o = xns_cons(x->vm, x->car, xns_append(x->cdr, y));
    xns_gc_unregister(x->vm, &y);
    xns_gc_unregister(x->vm, &x);
    return o;
}

size_t xns_len(xns_obj list){
    // hopefully this will prevent problems
    if (list->type == XNS_STRING || list->type == XNS_ARRAY) {
        return list->len;
    } else if (list->type != XNS_CONS){
        list->vm->error(list->vm, "Argument to xns_len is not a sequence (either a list, a string, or an array).", list);
        return 0;
    }
    size_t len = 0;
    while(!xns_nil(list)){
        list = list->cdr;
        len++;
    }
    return len;
}

xns_object *xns_make_fixnum(struct xns_vm *vm, long value){
    xns_obj obj = xns_alloc_object(vm, XNS_FIXNUM, sizeof(long));
    obj->fixnum = value;
    return obj;
}
xns_object *xns_make_double(struct xns_vm *vm, double value){
    xns_obj obj = xns_alloc_object(vm, XNS_DOUBLE, sizeof(double));
    obj->dval = value;
    return obj;
}
xns_object *xns_make_primitive(struct xns_vm *vm, xns_primitive value){
    xns_obj obj = xns_alloc_object(vm, XNS_PRIMITIVE, sizeof(xns_primitive));
    obj->primitive = value;
    return obj;
}
xns_object *xns_make_string(struct xns_vm *vm, char *value){
    size_t len = strlen(value) + 1;
    xns_obj obj = xns_alloc_object(vm, XNS_STRING, sizeof(size_t) + len);
    strncpy(obj->string, value, len);
    obj->string[len-1] = 0;
    obj->len = len-1;
    return obj;
}
xns_object *xns_make_array(struct xns_vm *vm, size_t len){
    size_t arrlen = sizeof(xns_obj*) * len;
    xns_obj obj = xns_alloc_object(vm, XNS_ARRAY, sizeof(size_t) + arrlen);
    memset((void*)obj->array, 0, arrlen);
    obj->length = len; // length and len are aliases
    return obj;
}
xns_object *xns_make_function(struct xns_vm *vm, xns_obj params, xns_obj body, xns_obj env){
    xns_gc_register(vm, &params);
    xns_gc_register(vm, &body);
    xns_gc_register(vm, &env);
    xns_obj obj = xns_alloc_object(vm, XNS_FUNCTION, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    xns_gc_unregister(vm, &params);
    xns_gc_unregister(vm, &body);
    xns_gc_unregister(vm, &env);
    return obj;
}
xns_object *xns_make_macro(struct xns_vm *vm, xns_obj params, xns_obj body, xns_obj env){
    xns_gc_register(vm, &params);
    xns_gc_register(vm, &body);
    xns_gc_register(vm, &env);
    xns_obj obj = xns_alloc_object(vm, XNS_MACRO, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    xns_gc_unregister(vm, &params);
    xns_gc_unregister(vm, &body);
    xns_gc_unregister(vm, &env);
    return obj;
}


/// CONVERSION
xns_object *xns_to_integer(struct xns_vm *vm, xns_obj value){
    (void) value;
    return vm->NIL; // TODO IMPLEMENT
    /*switch(value->type){
        case XNS_FIXNUM:
    }*/
}
xns_object *xns_to_real(struct xns_vm *vm, xns_obj value){
    (void) value;
    return vm->NIL;// TODO IMPLEMENT
}
xns_object *xns_to_double(struct xns_vm *vm, xns_obj value){
    switch (value->type){
        default:
            return vm->NIL;
        case XNS_INTEGER:
            return xns_make_double(vm, 0.0/0.0);
        case XNS_DOUBLE:
            return value;
        case XNS_FIXNUM:;
            long v = value->fixnum;
            return xns_make_double(vm, (double)v);
    }
}