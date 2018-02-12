#include "xns_lisp.h"
#include <sys/mman.h>
// create an xns_vm
struct xns_vm *xns_create_vm(size_t initial_heap_size){
    xns_vm *vm = calloc(1, sizeof(xns_vm));
    vm->frame = NULL;
    vm->symbols = NULL;
    vm->heap.size = (initial_heap_size < 1024*1024)? 1024*1024 : initial_heap_size;
    vm->heap.old_heap = NULL;
    vm->heap.current_heap = mmap(NULL, initial_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    vm->current_objectID = 0;
    vm->current_symbol = 0;
    vm->NIL = xns_intern(vm, "NIL");
    vm->T = xns_intern(vm, "T");
    // protect the VM from its own gc
    xns_gc_register(vm, &vm->symbols);
    xns_gc_register(vm, &vm->env);
    xns_gc_register(vm, &vm->NIL);
    xns_gc_register(vm, &vm->T);
    return NULL;//TODO implement
}
// destroy an xns_vm
void xns_destroy_vm(struct xns_vm *vm){
    // TODO IMPLEMENT
}

static struct xns_gcframe *makeframe(struct xns_vm *vm){
    struct xns_gcframe *frame = calloc(1, sizeof(struct xns_gcframe));
    frame->next = vm->frame;
    if(vm->frame){
        vm->frame->prev = frame;
    }else{
        vm->firstframe = frame;
    }
}

static void deleteframe(struct xns_gcframe *frame){
    if(frame->next){
        frame->next->prev = frame->prev;
    }
    if(frame->prev){
        frame->prev->next = frame->next;
    }
    free(frame);
}

//// C Local variable registration -- you don't want xns_objects moving without you knowing, right?
//// If you don't understand this please just use the handle api
/// CURRENTLY ORDER 2 * number of frames * frame_size = 2046 * nframes
// register a local variable -- this is primarily for internal purposes
void xns_gc_register(struct xns_vm *vm, struct xns_object **ptr){
    struct xns_gcframe *curr = vm->firstframe;
    if(!vm->frame){
        curr = makeframe(vm);
    }
    // try to find the pointer
    for(; curr; curr = curr->next){
        for (int i = 0; i < XNS_GCFRAME_SIZE; i++){
            if(curr->ptrs[i] == ptr){
                curr->counts[i] ++;
                return;
            }
        }
    }
    // try to find a free spot
    for(curr = vm->firstframe; curr; curr = curr->next){
        for(int i = 0; i < XNS_GCFRAME_SIZE; i++){
            if(curr->ptrs[i] == NULL){
                curr->ptrs[i] = ptr;
                curr->counts[i] = 1;
            }
        }
    }
    // make a new frame
    curr = makeframe(vm);
    curr->ptrs[0] = ptr;
    curr->counts[0] = 1;
}
// unregister a local variable -- if you forget to do this it will corrupt the stack on the next GC
void xns_gc_unregister(struct xns_vm *vm, struct xns_object **ptr){
    if(!vm->frame) return;
    struct xns_gcframe *curr = vm->firstframe; 
    // try to find the pointer
    for(; curr; curr = curr->next){
        for (int i = 0; i < XNS_GCFRAME_SIZE; i++){
            if(curr->ptrs[i] == ptr){
                curr->counts[i]--;
                // test to see if the pointer can be completely unregistered
                if(curr->counts[i] <= 0){
                    curr->ptrs[i] = NULL;
                    curr->counts[i] = 0;
                }
                return;
            }
        }
    }
}

void xns_gc_compactframe(struct xns_vm *vm){
    // TODO implement
}