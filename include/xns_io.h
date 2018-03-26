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
#ifndef XNS_IO_H
#define XNS_IO_H
// S-expression input and output
#include "xns_common.h"

char *xns_to_string(xns_obj object);
void  xns_print_object(xns_obj object);
// read an object from fp(this doesn't have to be a file on disk - lookup POSIX
// fmemopen(3) and open_memstream(3), GNU fopencookie(3), and BSD funopen(3). 
xns_object *xns_read_file(struct xns_vm *vm, FILE *fp);
// read an object from a buffer - this is actually implemented using fmemopen(3)
xns_object *xns_read_buffer(struct xns_vm *vm, char *memory, size_t length);
// read a whole file
xns_object *xns_read_whole_file(struct xns_vm *vm, FILE *fp);
// read a whole buffer
xns_object *xns_read_whole_buffer(struct xns_vm *vm, void *memory, size_t length);
#endif //XNS_IO_H