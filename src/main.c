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
#include <assert.h>
#define NOBJ 8//1024
int main(int argc, char**argv){
    struct xns_vm *vm = xns_create_vm(128);
    assert(vm);
    printf("Created VM.\n");
    xns_object *objects[NOBJ+1];
    xns_object *orig[NOBJ+1];
    memset(objects, 0, sizeof(objects));
    for(int i = 0; i < NOBJ+1; i++){
        xns_gc_register(vm, &objects[i]);
    }
    int diff_count = 0;
    // allocation test
    for(int i = 0; i < NOBJ; i++){
        //allocatge objects
        objects[i] = xns_alloc_object(vm, XNS_FIXNUM, sizeof(long));
        if(objects[i]->object_id == 3) printf("OBJ 3 IS HERE!\n");
        orig[i] = objects[i];
        objects[i]->fixnum = i;
        for(int j = 0; j < i+1; j++){
            if(orig[j] != objects[j]){
                #ifdef DEBUG_PRINT
                printf("Objects[%d] changed from %p to %p.\n", j, orig[j], objects[j]);
                #endif
                diff_count++;
            }
        }
    }
    objects[NOBJ] = objects[1];
    printf("Diff count = %d\n", diff_count);
    xns_vm_gc(vm, vm->heap.size);
    printf("Objects[NOBJ] is type=%d size=%lu value=%ld", objects[NOBJ]->type, objects[NOBJ]->size, objects[NOBJ]->fixnum);
    printf("VM NIL is %s %lu\n", vm->NIL->symname, vm->NIL->symid);
    xns_object *aa = xns_intern(vm, "NIL");
    printf("Symbols NIL is %s %lu\n", aa->symname, aa->symid);
    xns_object *sym = xns_intern(vm, "foo");
    printf("Created symbol foo\n");
    xns_gc_register(vm,  &sym);
    xns_assoc(vm->env, sym, objects[NOBJ - 3]);
    printf("Associated value\n");
    xns_object *o = xns_find(vm->env, sym);
    printf("Found object %u (type=%d size=%lu fixnum=%ld) for symbol %s\n", o->object_id, o->type, o->size, o->fixnum, sym->symname);
    // clear out objects so that the GC can clean up
    for(int i = 0 ; i < NOBJ+1; i++){
        xns_gc_unregister(vm, &objects[i]);
    }
    memset(objects, 0, sizeof(objects));
    // RPL -- READ PRINT LOOP (It's only missing EVAL!)
    while(!feof(stdin)){
        struct xns_object *rdobj = xns_read_file(vm, stdin);
        xns_print_object(rdobj);
    }
    return 0;
}