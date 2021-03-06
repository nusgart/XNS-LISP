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
#include <sys/mman.h>

static inline size_t roundup(size_t var, size_t size) {
    return (var + size - 1) & ~(size - 1);
}

// allocates space for an XNS-Object
xns_object *xns_alloc_object(struct xns_vm *vm, enum xns_type type, size_t size){
    fprintf(vm->debug, "---------------------------------------------\n");
    fprintf(vm->debug, "Originally requested %lu bytes\n", size);
    size = roundup(size, sizeof(void*));
    size += offsetof(xns_object, car);
    size = roundup(size, sizeof(void*));
    if(type == 0) abort();
    fprintf(vm->debug, "Allocating object of size %lu type %s (%d)\n", size, xns_type_to_string(type), type);
    // todo implement heap reclamation
    if(vm->heap.used + size > vm->heap.size){
        #ifndef NO_HEAP_RECLAIM
        xns_vm_gc(vm, vm->heap.used);
        #endif
        size_t hs = vm->heap.used << 1;
        if(hs < vm->heap.used + size){
            hs += size;
        }
        // this actually gives us more free space
        xns_vm_gc(vm, hs);
    }
    xns_obj obj = (struct xns_object*)(vm->heap.current_heap+vm->heap.used);
    obj->size = size;
    obj->type = type;
    obj->object_id = vm->current_objectID++;
    obj->vm = vm;
    vm->heap.allocs++;
    vm->heap.used += size;
    fprintf(vm->debug, "Currently used %lu bytes after making object %u\n", vm->heap.used, obj->object_id);
    fprintf(vm->debug, "---------------------------------------------\n\n");
    return obj;
}

// move an xns_object to the new heap
static struct xns_object *_xns_forward(struct xns_vm *vm, xns_obj obj){
    if(!obj){
        fprintf(vm->debug, "Not forwarding null pointer\n");
        return NULL;
    }
    // check to see if it is actually in the heap
    ptrdiff_t diff = (uint8_t*)obj - (uint8_t*) vm->heap.old_heap;
    if(diff < 0 || (size_t)diff > vm->heap.size){
        fprintf(stderr, "Object %p not in gc heap %p diff %ld\n", obj, vm->heap.current_heap, diff);
        fprintf(stderr, "Object info type=%d size=%lu id=%u\n", obj->type, obj->size, obj->object_id);
        abort();
        return obj;
    }
    // if the object was already moved, return the new object
    if(obj->type == XNS_MOVED){
        fprintf(vm->debug, "Object %u already moved\n", obj->object_id);
        return obj->ptr;
    }
    fprintf(vm->debug, "Forwarding object %u of type %d size %lu\n", obj->object_id, obj->type, obj->size);
    // otherwise, move the object to the new heap
    xns_obj nw = vm->scan2;
    memcpy(nw, obj, obj->size);
    vm->scan2 = (struct xns_object*)((uint8_t*)vm->scan2 + obj->size);
    ptrdiff_t d2 = (uint8_t*)vm->scan2 - (uint8_t*) vm->heap.current_heap;
    fprintf(vm->debug, "Scan2 has advanced %ld\n", d2);
    // make the old object a forwarder to the new one
    obj->type = XNS_MOVED;
    obj->ptr = nw;
    return nw;
}
void xns_print_heap(struct xns_vm *vm);

////INTERNAL
void xns_gc_compactframe(struct xns_vm *vm);

// use vm->frame to forward all of the roots and update them
static void _xns_forward_roots(struct xns_vm *vm){
    int cnt = 0, us = 0;
    long total_unused = 0;
    for(struct xns_gcframe *frame = vm->firstframe; frame; frame = frame->next){
        fprintf(vm->debug, "Handling %d-th frame\n", cnt++);
        us = 0;
        for(int i = 0; i < XNS_GCFRAME_SIZE; i++){
            // ignore empty entries
            if(!frame->ptrs[i]) continue;
            fprintf(vm->debug, "Forwarding root pointer %p (points to %p) with ref count %lu\n", frame->ptrs[i], *(frame->ptrs[i]), frame->counts[i]);
            *frame->ptrs[i] = _xns_forward(vm, *frame->ptrs[i]); 
            us++;
        }
        total_unused += XNS_GCFRAME_SIZE - us;
        fprintf(vm->debug, "%d entries used in frame %d\n", us, cnt-1);
    }
    // frame compaction
    if( (total_unused >  0.75 * XNS_GCFRAME_SIZE * cnt) && cnt > 4){
        xns_gc_compactframe(vm);
    }
    fprintf(vm->debug, "Finished -- had %d frames\n", cnt);
}

// trigger a garbage collection -- don't call this on your own! 
void xns_vm_gc(struct xns_vm *vm, size_t newsize){
    fprintf(vm->debug, "Collecting Garbage!\n");
    fprintf(stderr, "Collecting Garbage\n");
    xns_print_heap(vm);
    if(vm->gc_active){
        // todo error
        return;
    }
    vm->gc_active = true;
    // ensure the new heap is large enough
    if (newsize < vm->heap.used) {
        newsize = vm->heap.used + 2;
    }
    if (newsize < vm->heap.min_size) {
        newsize = vm->heap.min_size;
    }
    // sanity check
    if(newsize > PTRDIFF_MAX){
        fprintf(stderr, "XNS_Lisp: new heap size %lu exceeded PTRDIFF_MAX (%ld) !!!", newsize, PTRDIFF_MAX);
        abort();
    }
    fprintf(vm->debug, "Heap sizes old=%lu, new=%lu used=%lu\n", vm->heap.size, newsize, vm->heap.used);
    vm->heap.old_heap = vm->heap.current_heap;
    // allocate the new heap
    vm->heap.current_heap = mmap(NULL, newsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    memset(vm->heap.current_heap, 0, newsize);
    vm->scan1 = vm->scan2 = vm->heap.current_heap;
    _xns_forward_roots(vm);
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
            case XNS_RATIONAL:
            case XNS_SYMBOL:
            case XNS_GENSYM:
            case XNS_FOREIGN_FUNC:
            case XNS_FOREIGN_PTR:
            case XNS_HANDLE:
                break;
            /// forward the pointers contained by these types
            case XNS_CONS:
                fprintf(vm->debug, "Forwarding cons\n");
                vm->scan1->car = _xns_forward(vm, vm->scan1->car);
                vm->scan1->cdr = _xns_forward(vm, vm->scan1->cdr);
                break;
            case XNS_FUNCTION:
            case XNS_MACRO:
                fprintf(vm->debug, "Forwarding function/macro\n");
                vm->scan1->args = _xns_forward(vm, vm->scan1->args);
                vm->scan1->body = _xns_forward(vm, vm->scan1->body);
                vm->scan1->env  = _xns_forward(vm, vm->scan1->env);
                break;
            case XNS_ENV:
                fprintf(vm->debug, "Forwarding env\n");
                vm->scan1->vars = _xns_forward(vm, vm->scan1->vars);
                vm->scan1->parent = _xns_forward(vm, vm->scan1->parent);
                break;
            case XNS_ARRAY:
                fprintf(vm->debug, "Forwarding array\n");
                for(size_t idx = 0; idx < vm->scan1->length; idx++){
                    vm->scan1->array[idx] = _xns_forward(vm, vm->scan1->array[idx]);
                }
                break;
            case XNS_MAP:
                // this should be sufficient
                vm->scan1->bucket_array = _xns_forward(vm, vm->scan1->bucket_array);
                break;
            // nothing in the new heap should have been moved out yet!!!
            case XNS_MOVED:
                fprintf(vm->debug, "OBJECT ALREADY MOVED!!! %d!\n", vm->scan1->type);
                fprintf(stderr, "OBJECT ALREADY MOVED!!! %d!\n", vm->scan1->type);
                abort();
            default:
                fprintf(vm->debug, "INVALID OBJECT TYPE %d!\n", vm->scan1->type);
                fprintf(stderr, "INVALID OBJECT TYPE %d!\n", vm->scan1->type);
                abort();
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
    vm->heap.numGCs++;
    vm->gc_active = false;
    fprintf(vm->debug, "WE DIDN'T CRASH!!!\n");
}

// gives out an xns_handle -- this is an object that normal C code can handle
xns_object *xns_get_handle(struct xns_vm *vm, xns_obj obj){
    xns_obj o = calloc(1, sizeof(xns_object));
    o->vm = vm;
    o->type = XNS_HANDLE;
    o->size = sizeof(xns_object);
    o->object_id = (unsigned)-1; // invalid id
    o->foreign_pointer = obj;
    xns_gc_register(vm, &o->ptr);
    return o;
}
// destroy a handle -- until handles are destroyed, the object they point
// to can not be garbage collected
void xns_destroy_handle(struct xns_vm *vm, xns_obj handle){
    xns_gc_unregister(vm, &handle->ptr);
    handle->foreign_pointer = NULL;
    handle->vm = NULL;
    free(handle);
}

/// massive hack do not use
void xns_print_heap(struct xns_vm *vm){
    xns_obj ptr = vm->heap.current_heap;
    xns_obj end = (struct xns_object*)((uint8_t*)ptr+vm->heap.used);
    while(ptr < end){
        ptrdiff_t offset = (uint8_t*)ptr - (uint8_t*)vm->heap.current_heap;
        if(offset + ptr->size > vm->heap.used) break;
        fprintf(stderr, "Object at offset %ld has type %d size %lu id %u\n", offset, ptr->type, ptr->size, ptr->object_id);
        if(ptr->size == 0){
            break;
        }
        ptr = (struct xns_object*)((uint8_t*)ptr + ptr->size);
    }
}