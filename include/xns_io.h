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

char *xns_to_string(struct xns_object *object);
void  xns_print_object(struct xns_object *object);
// read an object from memory
struct xns_object *xns_read_memory(char *memory, size_t length);
struct xns_object *xns_read_file(FILE *fp);

#endif //XNS_IO_H