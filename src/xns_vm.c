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

/// FOR PRODUCTION DEFINE TO 1 MB -- 1048576
#define MIN_HEAP_SIZE 128

// create an xns_vm
struct xns_vm *xns_create_vm(size_t initial_heap_size){
    xns_vm *vm = calloc(1, sizeof(xns_vm));
    vm->frame = NULL;
    if(initial_heap_size < MIN_HEAP_SIZE){
        initial_heap_size = MIN_HEAP_SIZE;
    }
    // setup heap
    vm->heap.size = initial_heap_size;
    vm->heap.used = 0;
    vm->heap.allocs = 0;
    vm->heap.current_heap = mmap(NULL, initial_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    // initialize gc variables
    vm->heap.old_heap = NULL;
    vm->gc_active = false;
    vm->scan1 = NULL;
    vm->scan2 = NULL;
    // initialize object and symbol count
    vm->current_objectID = 0;
    vm->current_symbol = 0;
    // null out symbols and env 
    vm->symbols = NULL;
    vm->env = NULL;
    // we need to protect the VM from its own gc
    xns_gc_register(vm, &vm->symbols);
    // register symbols
    vm->NIL = xns_intern(vm, "NIL");
    asm volatile("": : :"memory");
    vm->symbols->cdr = vm->NIL;
    asm volatile("": : :"memory");
    xns_gc_register(vm, &vm->NIL);
    vm->T = xns_intern(vm, "T");
    xns_gc_register(vm, &vm->T);
    vm->env = xns_make_env(vm, NULL);
    xns_gc_register(vm, &vm->env);
    vm->Rparen = xns_intern(vm, ")");
    xns_gc_register(vm, &vm->Rparen);
    vm->Dot = xns_intern(vm, ".");
    xns_gc_register(vm, &vm->Dot);
    vm->Quote = xns_intern(vm, "quote");
    xns_gc_register(vm, &vm->Quote);
    return vm;
}
// destroy an xns_vm
void xns_destroy_vm(struct xns_vm *vm){
    // TODO IMPLEMENT
}

static struct xns_gcframe *makeframe(struct xns_vm *vm){
    struct xns_gcframe *frame = calloc(1, sizeof(struct xns_gcframe));
    frame->prev = vm->frame;
    if(vm->frame){
        printf("Making frame\n");
        vm->frame->next = frame;
        vm->frame = frame;
    }else{
        printf("Creating first gcframe\n");
        vm->firstframe = frame;
        vm->frame = frame;
    }
    return frame;
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
void xns_gc_register(struct xns_vm *vm, struct xns_object *const*ptr){
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
                curr->ptrs[i] = (struct xns_object**)ptr;
                curr->counts[i] = 1;
                //printf("Registered pointer %p at index %d\n", ptr, i);
                return;
            }
        }
    }
    // make a new frame
    curr = makeframe(vm);
    curr->ptrs[0] = (struct xns_object**)ptr;
    curr->counts[0] = 1;
}
// unregister a local variable -- if you forget to do this it will corrupt the stack on the next GC
void xns_gc_unregister(struct xns_vm *vm, struct xns_object *const*ptr){
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

// compact gc frames
void xns_gc_compactframe(struct xns_vm *vm){
    // TODO perhaps coalesce frames
    if(!vm->frame)return;
    struct xns_gcframe *curr;
    for(curr = vm->firstframe; curr;){
        bool clear = true;
        for(int i = 0; i < XNS_GCFRAME_SIZE; i++){
            if(curr->ptrs[i] && curr->counts[i] > 0){
                clear = false;
            }
        }
        if(clear){
            struct xns_gcframe *tmp = curr;
            curr = curr->next;
            deleteframe(tmp);
        }else{
            curr = curr->next;
        }
    }
}