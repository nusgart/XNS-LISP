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

#ifndef XNS_VM_H
#define XNS_VM_H

#include "xns_obj.h"

#define XNS_GCFRAME_SIZE 1023

/*
 * An xns_gcframe stores a single frame's worth of root pointers, which are the
 * pointers that the GC starts its scan for live objects from.  Additionally,
 * as XNS uses a precise copying GC, these pointers will be mutated to point
 * to the new location (which is why they are pointers to pointers).  Do not
 * mess with GC frames directly, just use the registration API.
 */ 
struct xns_gcframe{
    struct xns_object **ptrs[XNS_GCFRAME_SIZE];
    size_t counts[XNS_GCFRAME_SIZE];
    struct xns_gcframe *prev, *next;
};

// An XNS-Lisp Virtual machine: different VM's should be independent of each other.
// xns_vm is NOT currently threadsafe
struct xns_vm{
    //// Symbol and gensym
    // symbol and object id counters
    size_t current_symbol;
    size_t current_objectID;
    // the list of symbols
    struct xns_object *symbols;
    ////Environment
    struct xns_object *env;
    //// Global symbols
    struct xns_object *NIL;
    struct xns_object *T;
    struct xns_object *Dot;
    struct xns_object *Rparen;
    struct xns_object *Quote;
    //// gc info
    struct xns_object *scan1;
    struct xns_object *scan2;
    // stores the root pointers
    struct xns_gcframe *frame, *firstframe;
    //// heap data
    struct xns_heap{
        size_t size;
        size_t used;
        int allocs;
        void *current_heap;
        void *old_heap;
    } heap;
    bool gc_active;
};

/**
 * Creates an xns_vm with the specified initial heap size.
 * This (in order):
 * * creates the initial heap,
 * * initializes the current object and symbol ids to 0,
 * * initializes the first GC root frame,
 * * and sets up the initial symbols and the environment.
 */
struct xns_vm *xns_create_vm(size_t initial_heap_size);
/**
 * Destroys an xns_vm.
 */
void xns_destroy_vm(struct xns_vm *vm);
// trigger a garbage collection -- don't call this on your own! 
void xns_vm_gc(struct xns_vm *vm, size_t heapsize);
/************************************************************************************************
 * C Local variable registration API -- you don't want xns_objects moving without you knowing,
 * right? If you don't understand this please just use the handle api.  These functions are
 * primarily for internal purposes.
 ************************************************************************************************/
/**
 * Register a variable (technically any pointer to an xns_object) as a GC root
 * object with the Garbage Collector.  If you fail to do this with a normal
 * xns_object pointer, then the garbage collector WILL move it on collection
 * (this is how CheneyGC works), which will cause problems.
 * 
 * NOTE: The pointer being registered MUST be initialized to either NULL or
 * something in the GC heap!  Do NOT register uninitialized variables - THIS
 * WILL CAUSE SEEMINGLY INEXPLICABLE HEISENBUGS AND PHASE-OF-THE-MOON CRASHES
 * whenever a GC happens before the variable is intialized!  Once the root
 * pointer validity checker is removed from the GC, there will often be no
 * warning about this becuase at least GCC considers it possible for this
 * function to initialize *ptr (even though the type of ptr disallows that) and
 * thus does not issue any warning!
 */
void xns_gc_register(struct xns_vm *vm, struct xns_object *const*ptr);
/**
 * Unregister a local variable -- if you forget to do this it will corrupt the stack on the next GC
 */
void xns_gc_unregister(struct xns_vm *vm, struct xns_object *const*ptr);
#endif //XNS_VM_H