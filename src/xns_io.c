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

#define R(a) xns_gc_register(vm, &a)
#define U(a) xns_gc_unregister(vm, &a)
//TODO use sets to 
char *xns_to_string(xns_obj object){
    //TODO finish implementing
    char *desc, *fmt, *res;
    int as_len=0;
    xns_obj lm = NULL;
    if(!object) return strdup("#<NULLPTR>");
    xns_vm *vm = object->vm;
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
                vm->error(vm, "Out of memory when attempting to stringize double!", NULL);
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
                vm->error(vm, "Out of memory when attempting to stringize fixnum!", NULL);
            }
            return desc;
        case XNS_STRING:
            as_len = asprintf(&desc, "\"%s\"", object->string);
            if (as_len < 0) {
                vm->error(vm, "Out of memory when attempting to pretty-format string!", NULL);
            }
            return desc;
        // symbol / gensym
        case XNS_GENSYM:
        case XNS_SYMBOL:
            return strdup(object->symname);
        case XNS_CONS:
            desc = strdup("(");
            xns_obj obj = object;
            size_t len = 0;
            R(obj);
            R(object);
            while(true){
                char *o = desc;
                char *tmp = xns_to_string(obj->car);
                len = asprintf(&desc, "%s%s ", desc, tmp);
                free(tmp);
                if(len == (size_t)-1){
                    vm->error(vm, "Out of memory when attempting to stringize list!", NULL);
                }
                free(o);
                if(xns_nil(obj->cdr)) break;
                if(obj->cdr->type != XNS_CONS){
                    o = desc;
                    char *tm2 = xns_to_string(obj->cdr);
                    as_len = asprintf(&desc, "%s. %s)", desc, xns_to_string(obj->cdr));
                    free(tm2);
                    if(as_len == -1){
                        vm->error(vm, "Out of memory when attempting to stringize dotted list end!", NULL);
                    }
                    U(obj);
                    U(object);
                    free(o);
                    return desc;
                }
                obj = obj->cdr;
            }
            U(obj);
            U(object);
            desc[len - 1] = ')';
            return desc;
        case XNS_FUNCTION:
            R(object);
            lm = xns_cons(vm, object->args, object->body);
            R(lm);
            lm = xns_cons(vm, xns_intern(vm, "lambda"), lm);
            xns_gc_unregister(vm, &lm);
            U(lm);
            U(object);
            return xns_to_string(lm);
        case XNS_MACRO:
            R(object);
            lm = xns_cons(vm, object->args, object->body);
            R(lm);
            lm = xns_cons(vm, xns_intern(vm, "mlambda"), lm);
            xns_gc_unregister(vm, &lm);
            U(lm);
            U(object);
            return xns_to_string(lm);
        case XNS_ENV:
            // TODO print a list of variables and the values bound to them
            R(object);
            res = xns_to_string(object->vars);
            fmt = xns_to_string(object->parent);
            U(object);
            asprintf(&desc, "#<ENV vars=%s parent=%s>", res, fmt);
            free(res); free(fmt);
            return desc;
        case XNS_ARRAY:
            desc = strdup("");
            char *sep = "";
            R(object);
            assert(offsetof(xns_object, length) == offsetof(xns_object, len));
            for(size_t idx = 0; idx < object->length; idx++){
                char *d = xns_to_string(object->array[idx]);
                as_len = asprintf(&res, "%s%s%s", desc, sep, d);
                if(as_len == -1){
                    object->vm->error(object->vm, "Asprintf failed while attempting to stringize array!", NULL);
                }
                free(d);
                free(desc);
                sep = " ";
                desc = res;
            res = NULL;
            }
            U(object);
            asprintf(&res, "#(%s)", desc);
            free(desc);
            return res;
        case XNS_MOVED:
            fmt = "Moved Object --> [%s]";
            R(object);
            desc = xns_to_string(object->ptr);
            U(object);
            as_len = asprintf(&res, fmt, desc);
            if(as_len == -1){
                //TODO error
                object->vm->error(object->vm, "Out of memory when attempting to stringize moved object!", NULL);
            }
            free(desc);
            return res;
        default:
            // TODO ERROR
            vm->error(vm, "Unknown Object Type in xns_to_string!", object);
            return strdup("ERROR UNKNOWN OBJECT TYPE -- MEMORY CORRUPTED!!!!!!!!!!!!!!");
    }
    char *ret = NULL;
    as_len = asprintf(&ret, "#<Stringizing object at %p (type %s, length %lu)", object, xns_type_to_string(object->type), object->len);
    if (as_len == -1){
        vm->error(vm, "asprintf failed in xns_io.c xns_to_string unknown object!", NULL);
    }
    return ret;
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
            vm->error(vm, "List not terminated by end of file (or read error happened)", list);
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
            xns_obj last = xns_read_file(vm, fp);
            if(! xns_eq(last, vm->Rparen)){
                vm->error(vm, "More than one object follows . in list", last);
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
// windows is probably not going to work anyways...
#if defined(NEED_FMEMOPEN) || defined(__APPLE__) || defined(__WINDOWS__)
#include <unistd.h>
struct xns_priv_cookie{
    char *mem;
    size_t off;
    size_t len;
    int flags;
};
static ssize_t cread(void *cook, char *buf, size_t size){
    xns_priv_cookie *cookie = cook;
    if (cookie->off >= cookie->len) {
        return 0;
    }
    if(cookie->off + size > cookie->len){
        size = cookie->len - cookie->off;
    }
    memcpy(buf, cookie->mem + cookie->off, size);
    cookie->off += size;
    return size;
}

static ssize_t cwrite(void *cook, char *buf, size_t size){
    xns_priv_cookie *cookie = cook;
    if (cookie->off >= cookie->len) {
        return 0;
    }
    if(cookie->off + size > cookie->len){
        size = cookie->len - cookie->off;
    }
    memcpy(cookie->mem + cookie->off, buf, size);
    cookie->off += size;
    return size;
}

static int cseek(void *cook, off64_t *off, int whence){
    if (whence == SEEK_END || !off){
        return -1;
    }
    xns_priv_cookie *ck = cook;
    if (whence == SEEK_SET){
        if(*off > ck->len){
            *off = ck->len;
        }
        ck->off = *off;
        return 0;
    }
    if (whence == SEEK_CUR) {
        if (ck->off + *off > ck->len){
            *off = ck->len;
        } else *off += ck->off;
        ck->off = *off;
        return 0;
    }
    // invalid seek
    return -1;
}

static int cclose(void *cook){
    free(cook);
}

FILE *fmemopen(void *mem, size_t len, const char *mode){
    xns_priv_cookie *cookie = calloc(sizeof(xns_priv_cookie), 1);
    cookie->mem = mem;
    cookie->len = len;
    cookie->flags = ~0;
    // if(strstr(flags, "r")){
    //     cookie.flags |= O_RDONLY;
    // } // TODO implement flags

    cookie_io_functions_t ck = {};
    ck.read = cread;
    ck.write = cwrite;
    ck.close = cclose;
    ck.seek = cseek;
}
#endif

// read an object from memory 
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
        if (ferror(fp)) return NULL;
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
            // # is the read macro dispatch character, but read macros aren't implemented yet.
            case '#':
                if ((c = fgetc(fp)) == '('){
                    // read array
                    // this is a little bit of a hack
                    xns_obj lst = xns_read_list(vm, fp);
                    xns_gc_register(vm, &lst);
                    size_t len = xns_len(lst);
                    xns_obj obj = xns_make_array(vm, len);
                    xns_obj ptr = lst;
                    size_t idx = 0;
                    while(!xns_nil(ptr)){
                        if(ptr->type != XNS_CONS){
                            vm->error(vm, "NOT A CONS", ptr);
                        }
                        obj->array[idx++] = ptr->car;
                        ptr = ptr->cdr;
                    }
                    xns_gc_unregister(vm, &lst);
                    return obj;
                } else {
                    // not an array: set things back to they were before until read macros are implemented.
                    ungetc(c, fp);
                    c = '#';
                    break;
                }
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
                if (idx == currsize) {
                    buffer = realloc(buffer, 2 * currsize);
                    memset(buffer+currsize, 0, currsize);
                    currsize *= 2;
                }
                if( c == '.' || (base < 14 && (c == 'e' || c == 'E') ) || (base == 16 && (c == 'p' || c == 'P'))) {
                    isfloat = true;
                }
                c = getc(fp);
            } while (isdigit(c) || (base == 16 && strchr("AaBbCcDdEeFfPp", c)) || c == 'e' || c == 'E' || c == '.');
            ungetc(c, fp);
            xns_gc_register(vm, &obj);
            if (isfloat) {
                double d = strtod(buffer, NULL);
                obj = xns_make_double(vm, d);
            } else {
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