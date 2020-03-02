/* implementation of string object */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "object/strobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcString *new_string(char *s, size_t len) {
    MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ob->str = s;
    ob->isdyn = true;
    ITERABLE(ob)->length = len;
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

MxcString *new_string_copy(char *s, size_t len) {
    MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ob->str = malloc(sizeof(char) * (len + 1));
    memcpy(ob->str, s, len);
    ob->str[len] = '\0';

    ob->isdyn = true;
    ITERABLE(ob)->length = len;
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

MxcString *new_string_static(char *s, size_t len) {
    MxcString *ob = (MxcString *)Mxc_malloc(sizeof(MxcString));
    ITERABLE(ob)->index = 0;
    ITERABLE(ob)->next = NULL;
    ob->str = s;
    ob->isdyn = false;
    ITERABLE(ob)->length = len;
    OBJIMPL(ob) = &string_objimpl; 

    return ob;
}

MxcObject *string_copy(MxcObject *s) {
    MxcString *n = (MxcString *)Mxc_malloc(sizeof(MxcString));
    MxcString *old = (MxcString *)s;
    *n = *old; 

    char *olds = n->str;
    n->str = malloc(sizeof(char) * (ITERABLE(n)->length + 1));
    strcpy(n->str, olds);
    n->isdyn = true;

    return (MxcObject *)n;
}

void string_gc_mark(MxcObject *ob) {
    if(ob->marked) return;
    ob->marked = 1;
}

void str_guard(MxcObject *ob) {
    ob->gc_guard = 1;
}

void str_unguard(MxcObject *ob) {
    ob->gc_guard = 0;
}

void string_dealloc(MxcObject *s) {
    MxcString *str = (MxcString *)s;
    if(str->isdyn) {
        free(str->str);
    }
    Mxc_free(s);
}

MxcObject *str_index(MxcIterable *self, int64_t idx) {
    MxcString *str = (MxcString *)self;
    if(self->length <= idx) return NULL;

    return (MxcObject *)new_char_ref(&str->str[idx]);
}

MxcObject *str_index_set(MxcIterable *self, int64_t idx, MxcObject *a) {
    MxcString *str = (MxcString *)self;
    if(self->length <= idx) return NULL;
    str->str[idx] = ((MxcChar *)a)->ch;

    return a;
}

MxcString *str_concat(MxcString *a, MxcString *b) {
    size_t len = ITERABLE(a)->length + ITERABLE(b)->length;
    char *res = malloc(sizeof(char) * (len + 1));
    strcpy(res, a->str);
    strcat(res, b->str);

    return new_string(res, len);
}

void str_cstr_append(MxcString *a, char *b, size_t blen) {
    size_t len = ITERABLE(a)->length + blen;
    if(a->isdyn) {
        a->str = realloc(a->str, sizeof(char) * (len + 1));
    }
    else {
        char *buf = malloc(sizeof(char) * (len + 1));
        strcpy(buf, a->str);
        a->str = buf;
    }

    strcat(a->str, b);
    ITERABLE(a)->length = len;
    a->isdyn = true;
}

void str_append(MxcString *a, MxcString *b) {
    str_cstr_append(a, b->str, ITERABLE(b)->length);
}

MxcString *string_tostring(MxcObject *ob) {
    return (MxcString *)ob;
}

MxcObjImpl string_objimpl = {
    "string",
    string_tostring,
    string_dealloc,
    string_copy,
    string_gc_mark,
    str_guard,
    str_unguard,
    str_index,
    str_index_set,
};
