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
// S-expression input and output
#define _GNU_SOURCE
#include <stdio.h>
#include "xns_lisp.h"

char *xns_to_string(struct xns_object *object){
    //TODO finish implementing
    switch(object->type){
        case XNS_INVL:
            case XNS_INTEGER:
            case XNS_RATIONAL:
            case XNS_FOREIGN_FUNC:
            case XNS_FOREIGN_PTR:
                break;
            case XNS_DOUBLE:
                char *desc;
                asprintf(&desc, "%f", object->dval);
                return desc;
            case XNS_PRIMITIVE:
                return strdup("<#PRIMITIVE>");
            case XNS_HANDLE:
                return xns_to_string(object->ptr);
                break;
            case XNS_FIXNUM:
                char *desc;
                asprintf(&desc, "%ld", object->fixnum);
                return desc;
            case XNS_STRING:
                return strdup(object->string);
            // symbol / gensym
            case XNS_GENSYM:
            case XNS_SYMBOL:
                return strdup(object->symname);
            /// forward the pointers contained by these types
            case XNS_CONS:
                char *a1,a2, *desc;
                a1 = xns_to_string(object->car);
                a2 = xns_to_string(object->cdr);
                asprintf(&desc, "(%s . %s)", a1, a2);
                free(a1); free(a2);
                return desc;
            case XNS_FUNCTION:
            case XNS_MACRO:
                return xns_to_string(object->body);
            case XNS_ENV:
                // TODO print a list of variables and the values bound to them
                break;
            case XNS_MOVED:
                char *fmt = "Moved Object --> [%s]";
                char *res;
                char *desc = xns_to_string(object->ptr);
                asprintf(&res, fmt, desc);
                free(desc);
                return res;
            default:
                // TODO ERROR
                return strdup("ERROR UNKNOWN OBJECT TYPE -- MEMORY CORRUPTED!!!!!!!!!!!!!!");
    }
    return NULL;
}
void  xns_print_object(struct xns_object *object){
    char *a = xns_to_string(object);
    printf("%s\n",a);
    free(a);
}
// read an object from memory -- TODO implement these
struct xns_object *xns_read_memory(char *memory, size_t length);
struct xns_object *xns_read_file(FILE *fp);