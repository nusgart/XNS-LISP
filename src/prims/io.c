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
// XNS-Lisp primitive IO functions
// these are really builtins
#include "xns_lisp.h"

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)

xns_object *xns_prim_load   (struct xns_vm *vm, xns_obj env, xns_obj args){
    xns_obj res = eval(args->car, env);
    size_t len = res->len;
    char *name = strndup(res->string, len);
    FILE *fp = fopen(name, "r");
    xns_load_file(vm, vm->env, fp);
    fclose(fp);
    free(name);
    return vm->T;
}

