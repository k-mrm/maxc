#include "util.h"
#include "error/error.h"

Vector *New_Vector() {
    Vector *self = xmalloc(sizeof(Vector));

    self->data = xmalloc(sizeof(void *) * 16);
    self->len = 0;
    self->reserved = 16;

    return self;
}

Vector *New_Vector_With_Size(int size) {
    Vector *self = xmalloc(sizeof(Vector));

    self->data = xmalloc(sizeof(void *) * size);
    self->len = size;
    self->reserved = size;

    for(int i = 0; i < size; ++i) {
        self->data[i] = NULL;
    }

    return self;
}

void Delete_Vector(Vector *self) {
    free(self->data);
    free(self);
}

void vec_push(Vector *self, void *d) {
    if(self->len == self->reserved) {
        self->reserved *= 2;
        self->data = realloc(self->data, sizeof(void *) * self->reserved);
    }

    self->data[self->len++] = d;
}

void *vec_pop(Vector *self) {
#ifdef MXC_DEBUG
    if(self->len == 0) {
        error("vector is empty");
        return NULL;
    }
#endif

    return self->data[--self->len];
}

void *vec_last(Vector *self) { return self->data[self->len - 1]; }

Map *New_Map() {
    Map *self = xmalloc(sizeof(Map));

    self->key = New_Vector();
    self->value = New_Vector();

    return self;
}

void map_push(Map *self, void *key, void *value) {
    vec_push(self->key, key);
    vec_push(self->value, value);
}

void *map_search(Map *self, char *key) {
    int i = 0;
    for(; i < self->key->len; i++) {
        if(strlen(key) != strlen((char *)self->key->data[i]))
            continue;
        if(!strcmp(key, (char *)self->key->data[i]))
            break;
    }

    return self->value->data[i];
}

String *New_String() {
    String *self = xmalloc(sizeof(String));

    self->data = calloc(1, sizeof(char) * 16);
    self->len = 0;
    self->reserved = 16;

    return self;
}

void string_push(String *self, char v) {
    if(self->len == self->reserved) {
        self->reserved *= 2;
        self->data = realloc(self->data, sizeof(char) * self->reserved);
    }

    self->data[self->len++] = v;
}

char string_pop(String *self) {
    if(self->len == 0) {
        error("string is empty");
        return '\0';
    }

    self->data[self->len - 1] = 0;
    return self->data[--self->len];
}

int get_digit(int num) {
    char buf[100];

    return sprintf(buf, "%d", num);
}

char *read_file(char *path) {
    FILE *src_file = fopen(path, "r");
    if(!src_file) {
        return NULL;
    }

    fseek(src_file, 0, SEEK_END);
    size_t fsize = ftell(src_file);
    fseek(src_file, 0, SEEK_SET);

    char *src = xmalloc(sizeof(char) * (fsize + 1));

    if(fread(src, 1, fsize, src_file) < fsize) {
        error("Error reading file");
    }

    fclose(src_file);

    return src;
}
