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
    struct xns_object *p = vm->symbols;
    while(p && p != vm->NIL){
        if(!strcmp(p->car->symname, name)){
            return p->car;
        }
        p = p->cdr;
    }
    size_t size = strlen(name) + 1 + sizeof(size_t);
    struct xns_object *obj = xns_alloc_object(vm, XNS_SYMBOL, size);
    obj->symid = vm->current_symbol++;
    strcpy(obj->symname, name);
    printf("Added symbol %s with symid %lu objid %u\n", obj->symname, obj->symid, obj->object_id);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
// create a GENSYM -- a generated symbol that is guaranteed to have an unique value -- it will not be EQ to any other
//    symbol, even the SAME SYMBOL READ IN ANOTHER TIME: (EQ '#:G191 #:G191) evaluates to NIL!
xns_object *xns_gensym(xns_vm *vm){
    char gs[] = "#:G1234567890";
    struct xns_object *obj = xns_alloc_object(vm, XNS_GENSYM, sizeof(gs) + sizeof(size_t));
    char *gsym = obj->symname;
    obj->symid = vm->current_symbol++;
    snprintf(gsym, sizeof(gs), "#:G%lu", obj->symid);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
// are 
bool xns_eq(xns_object *a, xns_object *b){
    if(!a || !b) return !a && !b;
    if(a == b) return true;
    if(a->type != XNS_SYMBOL && a->type != XNS_GENSYM) return false;
    if(b->type != XNS_SYMBOL && b->type != XNS_GENSYM) return false;
    return a==b || (a->vm == b->vm && a->symid == b->symid);
}
// is obj nill? (Nill is the Lisp equivalent of NULL -- it is the empty list, false, and also a symbol)
bool xns_nil(xns_object *obj){
    return !obj || obj == obj->vm->NIL;
}
// environment
struct xns_object *xns_make_env(xns_vm *vm, xns_object *parent){
    xns_gc_register(vm, &parent);
    struct xns_object *obj = xns_alloc_object(vm, XNS_ENV, 2 * sizeof(xns_object *));
    printf("ENV Object %d has size %lu\n", obj->object_id, obj->size);
    obj->parent = parent? parent: vm->NIL;
    obj->vars = vm->NIL;
    xns_gc_unregister(vm, &parent);
    return obj;
}
/*
  An environment is implemented as an association list of symbols to values
 */
struct xns_object *xns_assoc(xns_object *env, xns_object *sym){
    struct xns_object *curr = env->vars;
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
struct xns_object *xns_set(xns_object *env, xns_object *sym, xns_object *value){
    xns_gc_register(env->vm, &env);
    xns_gc_register(env->vm, &sym);
    xns_gc_register(env->vm, &value);
    struct xns_object *pair = xns_cons(env->vm, sym, value);
    xns_gc_register(env->vm, &pair);
    env->vars = xns_cons(env->vm, pair, env->vars);
    xns_gc_unregister(env->vm, &pair);
    xns_gc_unregister(env->vm, &env);
    xns_gc_unregister(env->vm, &sym);
    xns_gc_unregister(env->vm, &value);
    return value;
}
// Cons Cells
xns_object *xns_car(xns_object *obj){
    if(!obj) return NULL;
    if(obj->type != XNS_CONS){
        if(obj == obj->vm->NIL){
            return obj;
        }
        // TODO error
        return NULL;
    }
    return obj->car;
}
struct xns_object *xns_cdr(xns_object *obj){
    if(!obj) return NULL;
    if(obj->type != XNS_CONS){
        if(obj == obj->vm->NIL){
            return obj;
        }
        // TODO error
        return NULL;
    }
    return obj->cdr;
}
struct xns_object *xns_cons(xns_vm *vm, xns_object *car, xns_object *cdr){
    xns_gc_register(vm, &car);
    xns_gc_register(vm, &cdr);
    xns_object *obj = xns_alloc_object(vm, XNS_CONS, 2 * sizeof(xns_object *));
    obj->car = car;
    obj->cdr = cdr;
    xns_gc_unregister(vm, &car);
    xns_gc_unregister(vm, &cdr);
    return obj;
}
struct xns_object *xns_nreverse(struct xns_object *list){
    struct xns_object *obj = list->vm->NIL;
    while(!xns_nil(list)){
        struct xns_object *hd = list;
        list = list->cdr;
        hd->cdr = obj;
        obj = hd;
    }
    return obj;
}

struct xns_object *xns_make_fixnum(struct xns_vm *vm, long value){
    struct xns_object *obj = xns_alloc_object(vm, XNS_FIXNUM, sizeof(long));
    obj->fixnum = value;
    return obj;
}
struct xns_object *xns_make_double(struct xns_vm *vm, double value){
    struct xns_object *obj = xns_alloc_object(vm, XNS_DOUBLE, sizeof(double));
    obj->dval = value;
    return obj;
}
struct xns_object *xns_make_primitive(struct xns_vm *vm, xns_primitive value){
    struct xns_object *obj = xns_alloc_object(vm, XNS_PRIMITIVE, sizeof(xns_primitive));
    obj->primitive = value;
    return obj;
}
struct xns_object *xns_make_string(struct xns_vm *vm, char *value){
    size_t len = strlen(value) + 1;
    struct xns_object *obj = xns_alloc_object(vm, XNS_STRING, len);
    strncpy(obj->string, value, len);
    obj->string[len-1] = 0; 
    return obj;
}
struct xns_object *xns_make_function(struct xns_vm *vm, struct xns_object *params, struct xns_object *body, struct xns_object *env){
    xns_gc_register(vm, &params);
    xns_gc_register(vm, &body);
    xns_gc_register(vm, &env);
    struct xns_object *obj = xns_alloc_object(vm, XNS_FUNCTION, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    xns_gc_unregister(vm, &params);
    xns_gc_unregister(vm, &body);
    xns_gc_unregister(vm, &env);
    return obj;
}
struct xns_object *xns_make_macro(struct xns_vm *vm, struct xns_object *params, struct xns_object *body, struct xns_object *env){
    xns_gc_register(vm, &params);
    xns_gc_register(vm, &body);
    xns_gc_register(vm, &env);
    struct xns_object *obj = xns_alloc_object(vm, XNS_MACRO, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    xns_gc_unregister(vm, &params);
    xns_gc_unregister(vm, &body);
    xns_gc_unregister(vm, &env);
    return obj;
}