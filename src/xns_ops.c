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

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)

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
    xns_obj currEnv = env;
    while(!xns_nil(currEnv)){
        xns_obj curr = currEnv->vars;
        while (!xns_nil(curr)){
            if (xns_eq(curr->car->car, sym)){
                curr->car->cdr = value;
                return curr->car;
            }
            curr = curr->cdr;
        }
        currEnv = currEnv->parent;
    }
    return xns_bind(env, sym, value);
}

xns_object *xns_bind(xns_obj env, xns_obj sym, xns_obj value){
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

struct xns_hash_region{
    size_t length;
    size_t used;
    uintptr_t *ptrs;
};

static inline unsigned long _xns_hash(xns_obj obj, struct xns_hash_region *region);

unsigned long xns_hash(xns_obj obj){
    struct xns_hash_region region = {};
    region.length = 128;
    region.used = 0;
    region.ptrs = calloc(128, sizeof(uintptr_t));
    unsigned long hash = _xns_hash(obj, &region);
    free(region.ptrs);
    return hash;
}
#define HASH_SEED 0x693f7257
/// modified djb hash function
static inline unsigned long _xns_hash(xns_obj obj, struct xns_hash_region *region){
    if (!obj) {
        return HASH_SEED;
    }
    for (size_t idx = 0; idx < region->used; idx++){
        if (region->ptrs[idx] == (uintptr_t)obj){
            return HASH_SEED;
        }
    }
    // follow pointer chain to direct object
    while (obj->type == XNS_MOVED || obj->type == XNS_HANDLE){
        region->ptrs[region->used++] = (uintptr_t)obj;
        if (region->used >= region->length) {
            region->ptrs = reallocarray(region->ptrs, 2 * region->length, sizeof(uintptr_t));
        }
        for (size_t idx = 0; idx < region->used; idx++){
            if (region->ptrs[idx] == (uintptr_t)obj){
                return HASH_SEED;
            }
        }  
        obj = obj->ptr;
    }
    // symbols
    if(obj->type == XNS_SYMBOL || obj->type == XNS_GENSYM){
        return obj->symid;
    }
    unsigned long hash = 0x693f7257; // randomly chosen starting value.
    if (obj->type == XNS_FOREIGN_FUNC) {
        return ((uintptr_t) obj->foreign_fcn) * 33 ^ hash;
    }
    // foreign types: just use
    if (obj->type == XNS_FOREIGN_PTR) {
        return hash * 33 ^ ((uintptr_t) obj->foreign_pointer);
    }
    if (obj->type == XNS_PRIMITIVE) {
        return hash * 33 ^ ((uintptr_t)obj->primitive);
    }
    if (obj->type == XNS_FUNCTION || obj->type == XNS_MACRO) {
        return hash * 33 ^ obj->object_id;
    }
    // numbers
    if (obj->type == XNS_FIXNUM) {
        return hash * 33 ^ obj->fixnum;
    }
    if (obj->type == XNS_DOUBLE) {
        union {
            double in;
            unsigned long bits;
        } un;
        un.in = obj->dval;
        //unsigned long bits = *(unsigned long *)&obj->dval;
        return hash * 33 ^ un.bits;
    }
    // array: take the hashes of the elements
    if (obj->type == XNS_ARRAY) {
        region->ptrs[region->used++] = (uintptr_t)obj;
        if (region->used >= region->length) {
            region->ptrs = reallocarray(region->ptrs, 2 * region->length, sizeof(uintptr_t));
        }
        hash = hash * 33 ^ obj->type;
        for(size_t idx = 0; idx < obj->length; idx++){
            hash = hash * 33 ^ _xns_hash(obj->array[idx], region);
        }
        return hash;
    }
    // conses: take the hashes of the cars
    if (obj->type == XNS_CONS) {
        hash = hash * 33 ^ obj->type;
        while (!xns_nil(obj) && obj->type == XNS_CONS) {
            region->ptrs[region->used++] = (uintptr_t)obj;
            if (region->used >= region->length) {
                region->ptrs = reallocarray(region->ptrs, 2 * region->length, sizeof(uintptr_t));
            }
            for (size_t idx = 0; idx < region->used; idx++){
                if (region->ptrs[idx] == (uintptr_t)obj){
                    return HASH_SEED;
                }
            }
            hash = hash * 33 ^ _xns_hash(obj->car, region);
            obj = obj->cdr;
        }
        // handle dotted pairs
        if (!xns_nil(obj)) {
            hash = hash * 33 ^ _xns_hash(obj, region);
        }
        return hash;
    }
    // otherwise, hash the object's bytes
    unsigned char *bytes = (unsigned char *)obj;
    size_t length = obj->size;
    if (obj->type == XNS_STRING){
        hash ^= obj->type;
        bytes = (unsigned char *)obj->string;
        length = obj->len;
    }
    for(size_t idx = 0; idx < length; idx++){
        hash = hash * 33 ^ bytes[idx];
    }
    return hash;
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

xns_object *xns_make_integer(struct xns_vm *vm, long value){
    (void) value;
    xns_obj obj = xns_alloc_object(vm, XNS_INTEGER, 256);
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
    R(params); R(body); R(env);
    xns_obj obj = xns_alloc_object(vm, XNS_FUNCTION, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    U(params); U(body); U(env);
    return obj;
}
xns_object *xns_make_macro(struct xns_vm *vm, xns_obj params, xns_obj body, xns_obj env){
    R(params); R(body); R(env);
    xns_obj obj = xns_alloc_object(vm, XNS_MACRO, 3 * sizeof(struct xns_object*));
    obj->args = params;
    obj->body = body;
    obj->env = env;
    U(params); U(body); U(env);
    return obj;
}


/// CONVERSION
xns_object *xns_to_integer(struct xns_vm *vm, xns_obj value){
    switch(value->type){
        case XNS_FIXNUM:
            return xns_make_integer(vm, value->fixnum);
        case XNS_INTEGER:
            return value;
        default:
            break;
    }
    return vm->NIL;
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

xns_object *xns_to_fixnum(struct xns_vm *vm, xns_obj value){
    switch (value->type){
        default:
            return vm->NIL;
        case XNS_INTEGER:
            // TODO
            return xns_make_fixnum(vm, (long)value->integer_implementation);
        case XNS_DOUBLE:
            return xns_make_fixnum(vm, (long)value->dval);
        case XNS_FIXNUM:;
            return value;
    }
}
// hash map
// create a map.
struct xns_object *xns_create_map(struct xns_vm *vm, size_t initalBuckets){
    xns_obj array = xns_make_array(vm, initalBuckets);
    R(array);
    xns_obj map = xns_alloc_object(vm, XNS_MAP, sizeof(intptr_t) * 4);
    map->nBuckets = initalBuckets;
    map->nItems = 0;
    map->load_factor = 0.75;
    map->bucket_array = array;
    map->useEq = false;
    U(array);
    return map;
}
// does the map contain key?
bool xns_map_contains(xns_obj map, xns_obj key){
    size_t hash = xns_hash(key) % map->nBuckets;
    xns_obj cell = map->bucket_array->array[hash];
    while(!xns_nil(cell)) {
        bool equal;
        if(map->useEq){
            equal = cell->array[0]->object_id == key->object_id;
        } else {
            equal = !xns_nil(xns_equal(map->vm, NULL, cell->array[0], key));
        }
        if (equal){
            return true;
        }
        cell = cell->array[2];
    }
    return false;
}
// Get an object from the map.  
xns_object *xns_map_get(xns_obj map, xns_obj key){
    size_t hash = xns_hash(key) % map->nBuckets;
    xns_obj cell = map->bucket_array->array[hash];
    while(!xns_nil(cell)) {
        bool equal;
        if(map->useEq){
            equal = cell->array[0]->object_id == key->object_id;
        } else {
            equal = !xns_nil(xns_equal(map->vm, NULL, cell->array[0], key));
        }
        if (equal){
            return cell->array[1];
        }
        cell = cell->array[2];
    }
    return map->vm->NIL;
}
// put an object in the map.  This may increase 
void xns_map_put(xns_obj map, xns_obj key, xns_obj val){
    struct xns_vm *vm = map->vm;
    R(map); R(key); R(val);
    map->nItems++;
    // expand the map if there are too many items
    if (map->nItems / map->nBuckets > map->load_factor) {
        xns_map_expand(map, map->nBuckets * 2);
    }
    size_t hash = xns_hash(key) % map->nBuckets;
    xns_obj arr = xns_make_array(vm, 3);
    arr->array[0] = key;
    arr->array[1] = val;
    arr->array[2] = map->bucket_array->array[hash];
    map->bucket_array->array[hash] = arr;
    U(map); U(key); U(val);
}

void xns_map_remove(xns_obj map, xns_obj key){
    size_t hash = xns_hash(key) % map->nBuckets;
    // remove the object from the chain
    xns_obj *overwrite = &(map->bucket_array->array[hash]);
    xns_obj cell = *overwrite;
    while(!xns_nil(cell)) {
        bool equal;
        if(map->useEq){
            equal = cell->array[0]->object_id == key->object_id;
        } else {
            equal = !xns_nil(xns_equal(map->vm, NULL, cell->array[0], key));
        }
        if (equal){
            *overwrite = cell->array[2];
        }
        overwrite = &cell->array[2];
        cell = cell->array[2];
    }
}

// forcibly expand the map to a larger size.
bool xns_map_expand(xns_obj map, size_t newBuckets){
    struct xns_vm *vm = map->vm;
    R(map);
    xns_obj buckets = map->bucket_array;
    R(buckets);
    map->bucket_array = xns_make_array(vm, newBuckets);
    for (size_t idx = 0; idx < buckets->length; idx++){
        xns_obj cell = buckets->array[idx];
        R(cell);
        while (!xns_nil(cell)){
            xns_map_put(map, cell->array[0], cell->array[1]);
            cell = cell->array[2];
        }
        U(cell);
    }
    map->nBuckets = newBuckets;
    U(map);
    U(buckets);
    return true;
}