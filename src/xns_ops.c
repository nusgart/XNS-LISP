#include "xns_lisp.h"

//symbols and gensyms
xns_object *xns_intern(xns_vm *vm, char *name){
    for(struct xns_object *p = vm->symbols; p != vm->NIL; p = p->cdr){
        if(!strcmp(p->car->symname, name)){
            return p->car;
        }
    }
    size_t size = strlen(name) + 1 + sizeof(size_t);
    struct xns_object *obj = xns_alloc_object(vm, XNS_SYMBOL, size);
    obj->symid = vm->current_symbol++;
    strcpy(obj->symname, name);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
xns_object *xns_gensym(xns_vm *vm){
    char gs[] = "G:1234567890";
    struct xns_object *obj = xns_alloc_object(vm, XNS_GENSYM, sizeof(gs) + sizeof(size_t));
    char *gsym = obj->symname;
    obj->symid = vm->current_symbol++;
    snprintf(gsym, sizeof(gs), "G:%lu", obj->symid);
    xns_gc_register(vm, &obj);
    vm->symbols = xns_cons(vm, obj, vm->symbols);
    xns_gc_unregister(vm, &obj);
    return obj;
}
bool xns_eq(xns_object *a, xns_object *b){
    if(!a || !b) return !a && !b;
    if(a == b) return true;
    if(a->type != XNS_SYMBOL && a->type != XNS_GENSYM) return false;
    if(b->type != XNS_SYMBOL && b->type != XNS_GENSYM) return false;
    return a==b || (a->vm == b->vm && a->symid == b->symid);
}
bool xns_nil(xns_object *obj);
// environment
xns_object *xns_make_env(xns_vm *vm, xns_object *parent){
    
}
xns_object *xns_find(xns_object *env, xns_object *sym);
xns_object *xns_assoc(xns_object *env, xns_object *sym, xns_object *value);
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
xns_object *xns_cdr(xns_object *obj){
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
xns_object *xns_cons(xns_vm *vm, xns_object *car, xns_object *cdr){
    xns_gc_register(vm, &car);
    xns_gc_register(vm, &cdr);
    xns_object *obj = xns_alloc_object(vm, XNS_CONS, 0);
    obj->car = car;
    obj->cdr = cdr;
    xns_gc_unregister(vm, &car);
    xns_gc_unregister(vm, &cdr);
    return obj;
}