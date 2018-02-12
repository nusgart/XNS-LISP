#include "xns_lisp.h"
#include <sys/mman.h>

static inline size_t roundup(size_t var, size_t size) {
    return (var + size - 1) & ~(size - 1);
}

// allocates space for an XNS-Object
struct xns_object *xns_alloc_object(struct xns_vm *vm, enum xns_type type, size_t size){
    size = roundup(size, sizeof(void*));
    size += offsetof(xns_object, vm);
    size = roundup(size, sizeof(void*));
    // todo implement heap reclamation
    if(vm->heap.used + size > vm->heap.size){
        size_t hs = vm->heap.used << 1;
        if(hs < vm->heap.used + size){
            hs += size;
        }
        // this actually gives us more free space
        xns_vm_gc(vm, 2 * vm->heap.used);
    }
    struct xns_object *obj = (struct xns_object*)(vm->heap.current_heap+vm->heap.used);
    obj->size = size;
    obj->type = type;
    obj->object_id = vm->current_objectID++;
    vm->heap.used += size;
    return obj;
}

// move an xns_object to the new heap
static struct xns_object *_xns_forward(struct xns_vm *vm, struct xns_object *obj){
    // check to see if it is actually in the heap
    ptrdiff_t diff = (uint8_t*)obj - (uint8_t*) vm->heap.old_heap;
    if(diff < 0 || diff > vm->heap.size){
        return obj;
    }
    // if the object was already moved, return the new object
    if(obj->type == XNS_MOVED){
        return obj->ptr;
    }
    // otherwise, move the object to the new heap
    struct xns_object *nw = vm->scan2;
    memcpy(nw, obj, obj->size);
    vm->scan2 = (struct xns_object*)((uint8_t*)vm->scan2 + obj->size);
    // make the old object a forwarder to the new one
    obj->type = XNS_MOVED;
    obj->ptr = nw;
    return nw;
}

// use vm->frame to forward all of the roots and update them
static void _xns_forward_roots(struct xns_vm *vm){
    for(struct xns_gcframe *frame = vm->firstframe; frame; frame = frame->next){
        for(int i = 0; i < XNS_GCFRAME_SIZE; i++){
            // ignore empty entries
            if(!frame->ptrs[i]) continue;
            *frame->ptrs[i] = _xns_forward(vm, *frame->ptrs[i]); 
        }
    }
}

// trigger a garbage collection -- don't call this on your own! 
void xns_vm_gc(struct xns_vm *vm, size_t newsize){
    if(vm->gc_active){
        // todo error
        return;
    }
    vm->gc_active = true;
    // ensure the new heap is large enough
    if(newsize < vm->heap.used) newsize = vm->heap.used;
    vm->heap.old_heap = vm->heap.current_heap;
    // allocate the new heap
    vm->heap.current_heap = mmap(NULL, newsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    vm->scan1 = vm->scan2 = vm->heap.current_heap;
    // copy all of the 
    while(vm->scan1 < vm->scan2){
        switch(vm->scan1->type){
            // none of these types can contain a pointer to a gc-managed object
            case XNS_INVL:
            case XNS_PRIMITIVE:
            case XNS_FIXNUM:
            case XNS_INTEGER:
            case XNS_DOUBLE:
            case XNS_STRING:
            case XNS_MOVED:
            case XNS_RATIONAL:
            case XNS_SYMBOL:
            case XNS_GENSYM:
            case XNS_FOREIGN_FUNC:
            case XNS_FOREIGN_PTR:
            case XNS_HANDLE:
                break;
            /// forward the pointers contained by these types
            case XNS_CONS:
                vm->scan1->car = _xns_forward(vm, vm->scan1->car);
                vm->scan1->cdr = _xns_forward(vm, vm->scan1->cdr);
                break;
            case XNS_FUNCTION:
            case XNS_MACRO:
                vm->scan1->args = _xns_forward(vm, vm->scan1->args);
                vm->scan1->body = _xns_forward(vm, vm->scan1->body);
                vm->scan1->env  = _xns_forward(vm, vm->scan1->env);
                break;
            case XNS_ENV:
                vm->scan1->vars = _xns_forward(vm, vm->scan1->vars);
                vm->scan1->parent = _xns_forward(vm, vm->scan1->parent);
                break; 
        }
        vm->scan1 = (struct xns_object*)((uint8_t*)vm->scan1 +vm->scan1->size);
    }
    // release the old heap
    munmap(vm->heap.old_heap, vm->heap.size);
    vm->heap.old_heap = NULL;
    // update the size and used bytes information
    vm->heap.size = newsize;
    ptrdiff_t diff = (uint8_t*)vm->scan1 - (uint8_t*)vm->heap.current_heap;
    vm->heap.used = (size_t)diff;
    vm->gc_active = false;
}

// gives out an xns_handle -- this is an object that normal C code can handle
xns_object *xns_get_handle(struct xns_vm *vm, struct xns_object *obj){
    xns_object *o = calloc(1, sizeof(xns_object));
    o->vm = vm;
    o->type = XNS_HANDLE;
    o->size = sizeof(xns_object);
    o->object_id = (unsigned)-1; // invalid id
    o->foreign_pointer = obj;
    xns_gc_register(vm, (struct xns_object**)&o->foreign_pointer);
}
// destroy a handle -- until handles are destroyed
void xns_destroy_handle(struct xns_vm *vm, struct xns_object *handle){
    xns_gc_unregister(vm, (struct xns_object**)&handle->foreign_pointer);
    handle->foreign_pointer = NULL;
    handle->vm = NULL;
    free(handle);
}