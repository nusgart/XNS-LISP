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
#ifndef XNS_MAP_H
#define XNS_MAP_H
#include "xns_lisp.h"
// create a map.
struct xns_object *xns_create_map(size_t initalBuckets);
// destroy a map.
void xns_delete_map(xns_obj map);
// does the map contain key?
bool xns_map_contains(xns_obj map, xns_obj key);
// Get an object from the map.  
xns_object *xns_map_get(xns_obj map, xns_obj key);
// put an object in the map.  This may increase 
void xns_map_put(xns_obj map, xns_obj key, xns_obj val);
// forcibly expand the map to a larger size.
bool xns_map_expand(xns_obj map, size_t newBuckets);
#endif