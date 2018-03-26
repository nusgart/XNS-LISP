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

#pragma once
#ifndef XNS_COMMON_H
#define XNS_COMMON_H
// common includes, forward declarations, ....
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef struct xns_object xns_object;
#ifndef XNS_NO_VOLATILE
typedef struct xns_object *volatile xns_obj;
#else
typedef struct xns_object *xns_obj;
#endif
typedef struct xns_vm xns_vm;
typedef struct xns_object *(*xns_primitive)(struct xns_vm *vm, xns_obj env, xns_obj obj); 
#endif //XNS_COMMON_H