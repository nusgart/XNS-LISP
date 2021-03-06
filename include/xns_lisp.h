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

#ifndef XNS_LISP_H
#define XNS_LISP_H
// include all xns headers
#include "xns_common.h"
#include "xns_obj.h"
#include "xns_vm.h"
#include "xns_ops.h"
#include "xns_io.h"
#include "xns_prims.h"
#include "xns_eval.h"
char *xns_type_to_string(enum xns_type type);
#endif //XNS_LISP_H