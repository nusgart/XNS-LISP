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
#ifndef XNS_EVAL_H
#define XNS_EVAL_H
/// the evalulator
#include "xns_common.h"

struct xns_object *eval(struct xns_object *obj, struct xns_object *env);
struct xns_object *evalis(struct xns_object *obj, struct xns_object *env);
struct xns_object *apply(struct xns_object *obj, struct xns_object *env, struct xns_object *args);
struct xns_object *eval_cond(struct xns_object *obj, struct xns_object *env);
#endif //XNS_EVAL_H