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
#include <ctype.h>
#include "xns_lisp.h"

char *xns_to_string(xns_obj object){
    //TODO finish implementing
    char *desc, *fmt, *res;
    int as_len=0;
    if(!object) return strdup("#<NULLPTR>");
    switch(object->type){
        case XNS_INVL:
            case XNS_INTEGER:
            case XNS_RATIONAL:
                break;
            case XNS_FOREIGN_FUNC:
                return strdup ("#<FOREIGN_FCN>");
            case XNS_FOREIGN_PTR:
                return strdup("#<FOREIGN_PTR>");
            case XNS_DOUBLE:
                as_len = asprintf(&desc, "%f", object->dval);
                if(as_len == -1){
                    //TODO error
                }
                return desc;
            case XNS_PRIMITIVE:
                return strdup("#<PRIMITIVE>");
            case XNS_HANDLE:
                return xns_to_string(object->ptr);
                break;
            case XNS_FIXNUM:
                as_len = asprintf(&desc, "%ld", object->fixnum);
                if(as_len == -1){
                    //TODO error
                }
                return desc;
            case XNS_STRING:
                as_len = asprintf(&desc, "\"%s\"", object->string);
                return desc;
            // symbol / gensym
            case XNS_GENSYM:
            case XNS_SYMBOL:
                return strdup(object->symname);
            /// forward the pointers contained by these types
            #ifdef BAD_PRINT
            case XNS_CONS:
                a1 = xns_to_string(object->car);
                a2 = xns_to_string(object->cdr);
                as_len = asprintf(&desc, "(%s . %s)", a1, a2);
                if(as_len == -1){
                    //TODO error
                }
                free(a1); free(a2);
                return desc;
            #else
            case XNS_CONS:
            desc = strdup("(");
           xns_obj obj = object;
            size_t len = 0;
            while(true){
                char *o = desc;
                char *tmp = xns_to_string(obj->car);
                len = asprintf(&desc, "%s%s ", desc, tmp);
                free(tmp);
                if(len == (size_t)-1){
                    //TODO error
                }
                free(o);
                if(xns_nil(obj->cdr)) break;
                if(obj->cdr->type != XNS_CONS){
                    o = desc;
                    char *tm2 = xns_to_string(obj->cdr);
                    as_len = asprintf(&desc, "%s . %s)", desc, xns_to_string(obj->cdr));
                    free(tm2);
                    if(as_len == -1){
                        //TODO error
                    }
                    free(o);
                    return desc;
                }
                obj = obj->cdr;
            }
            desc[len - 1] = ')';
            return desc;
            #endif
            case XNS_FUNCTION:
            case XNS_MACRO:
                return xns_to_string(object->body);
            case XNS_ENV:
                // TODO print a list of variables and the values bound to them
                res = xns_to_string(object->vars);
                fmt = xns_to_string(object->parent);
                asprintf(&desc, "#<ENV vars=%s parent=%s>", res, fmt);
                free(res); free(fmt);
                return desc;
            case XNS_MOVED:
                fmt = "Moved Object --> [%s]";
                desc = xns_to_string(object->ptr);
                as_len = asprintf(&res, fmt, desc);
                if(as_len == -1){
                    //TODO error
                }
                free(desc);
                return res;
            default:
                // TODO ERROR
                return strdup("ERROR UNKNOWN OBJECT TYPE -- MEMORY CORRUPTED!!!!!!!!!!!!!!");
    }
    return NULL;
}

#include "typestr.inc"

char *xns_type_to_string(enum xns_type type){
    return xns_type_strs[type];
}

void  xns_print_object(xns_obj object){
    char *a = xns_to_string(object);
    printf("%s\n",a);
    free(a);
}
static xns_object *xns_read_list(struct xns_vm *vm, FILE*fp){
    xns_obj list=NULL, car=NULL, cdr=NULL;
    xns_gc_register(vm, &car);
    xns_gc_register(vm, &cdr);
    xns_gc_register(vm, &list);
    list = vm->NIL;
    while(true){
        car = xns_read_file(vm, fp);
        if(!car){
            // TODO error
            break;
        }
        // handle list end
        if(xns_eq(car, vm->Rparen)){
            list = xns_nreverse(list);
            break;
        }
        // handle dotted pair
        if(xns_eq(car, vm->Dot)){
            cdr = xns_read_file(vm, fp);
            if(! xns_eq(xns_read_file(vm, fp), vm->Rparen)){
                // TODO error
            }
            xns_obj end = list;
            list = xns_nreverse(list);
            end->cdr = cdr;
            break;
        }
        list = xns_cons(vm, car, list);
    }
    xns_gc_unregister(vm, &car);
    xns_gc_unregister(vm, &cdr);
    xns_gc_unregister(vm, &list);
    return list;
}
// read an object from memory -- TODO implement these
xns_object *xns_read_memory(struct xns_vm *vm,char *memory, size_t length){
    FILE *file = fmemopen(memory, length, "r");
    xns_obj obj = xns_read_file(vm, file);
    fclose(file);
    return obj;
}
static int look(FILE*fp){
    int l = getc(fp);
    ungetc(l, fp);
    return l;
}

static const char symbol_chars[] = "~!@#$%^&*-_=+:/?<>";

xns_object *xns_read_file(struct xns_vm *vm, FILE *fp){
    // TODO
    xns_obj obj=NULL;
    char *buffer = NULL;
    size_t currsize = 256;
    size_t idx = 0;
    while (true){
        int c;
        st: c = getc(fp);
        if(isspace(c) || iscntrl(c)){ continue;}
        switch(c){
            case EOF:
                return NULL;
            case ';':
                // skip line
                while((c = fgetc(fp)) != '\n'){
                    if(c == EOF) return NULL;
                }
                goto st;
            case '(':
                return xns_read_list(vm, fp);
            case ')':
                return vm->Rparen;
            case '.':
                return vm->Dot;
            case '"':
                buffer = calloc(256, 1);
                while((c = getc(fp)) != '"'){
                    buffer[idx++] = c;
                    if(idx == currsize){
                        buffer = realloc(buffer, currsize * 2);
                        memset(buffer+currsize, 0, currsize);
                        currsize *= 2;
                    }
                }
                obj = xns_make_string(vm, buffer);
                free(buffer);
                return obj;
            case '\'':
                xns_gc_register(vm, &obj);
                obj = xns_read_file(vm, fp);
                obj = xns_cons(vm, obj, vm->NIL);
                obj = xns_cons(vm, vm->Quote, obj);
                xns_gc_unregister(vm, &obj);
                return obj;
            default: break;
        }
        buffer = calloc(256, 1);
        // read a number
        if(isdigit(c) || (c == '-' && isdigit(look(fp)) ) ){
            int base = 10;
            bool isfloat = false;
            switch(look(fp)){
                case 'x': base = 16; break;
                case 'o': base = 8; break;
                case 'b': base = 2; break;
            }
            do{
                buffer[idx++] = c;
                if(idx == currsize){
                    buffer = realloc(buffer, 2 * currsize);
                    memset(buffer+currsize, 0, currsize);
                    currsize *= 2;
                }
                if(c == '.' || (base < 14 && (c == 'e' || c == 'E') ) || (base == 16 && (c == 'p' || c == 'P'))){
                    isfloat = true;
                }
                c = getc(fp);
            } while(isdigit(c) || (base == 16 && strchr("AaBbCcDdEeFfPp", c)) || c == 'e' || c == 'E' || c == '.');
            ungetc(c, fp);
            xns_gc_register(vm, &obj);
            if(isfloat){
                double d = strtod(buffer, NULL);
                obj = xns_make_double(vm, d);
            }else{
                long l = strtol(buffer, NULL, base);
                obj = xns_make_fixnum(vm, l);
            }
            free(buffer);
            xns_gc_unregister(vm, &obj);
            return obj;
        }
        if(!isalnum(c) && !strchr(symbol_chars, c)){
            free(buffer);
            continue;
        }
        // read a symbol
        while(isalnum(c) || strchr(symbol_chars, c)){
            buffer[idx++] = c;
            if(idx == currsize){
                buffer = realloc(buffer, 2 * currsize);
                memset(buffer+currsize, 0, currsize);
                currsize *= 2;
            }
            c = getc(fp); 
        }
        ungetc(c, fp);
        obj = xns_intern(vm, buffer);
        free(buffer);
        return obj;
    }
}

// read a whole file
xns_object *xns_read_whole_file(struct xns_vm *vm, FILE *fp){
    xns_obj obj=vm->NIL;
    xns_gc_register(vm, &obj);
    while(!feof(fp)){
        obj = xns_cons(vm, xns_read_file(vm, fp), obj);
    }
    xns_gc_unregister(vm, &obj);
    return obj;
}

// read a whole buffer
xns_object *xns_read_whole_buffer(struct xns_vm *vm, void *memory, size_t length){
    FILE *file = fmemopen(memory, length, "r");
    xns_obj obj = xns_read_whole_file(vm, file);
    fclose(file);
    return obj;
}