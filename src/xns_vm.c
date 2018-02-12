#include "xns_vm.h"

// create an xns_vm
struct xns_vm *xns_create_vm(size_t initial_heap_size){
    return NULL;//TODO implement
}
// destroy an xns_vm
void xns_destroy_vm(struct xns_vm *vm){
    // TODO IMPLEMENT
}
// trigger a garbage collection -- don't call this on your own! 
void xns_vm_gc(struct xns_vm *vm){
    // todo implement
}
//// C Local variable registration -- you don't want xns_objects moving without you knowing, right?
//// If you don't understand this please just use the handle api
// register a local variable -- this is primarily for internal purposes
void xns_gc_register(void **ptr){
    // Todo implement
}
// unregister a local variable -- if you forget to do this it will corrupt the stack on the next GC
void xns_gc_unregister(void **ptr){
   // TODO implement 
}