#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "error/error.h"

Vector *new_vector() {
  Vector *self = xmalloc(sizeof(Vector));

  self->data = xmalloc(sizeof(void *) * 16);
  self->len = 0;
  self->reserved = 16;

  return self;
}

Vector *new_vector_capa(int size) {
  Vector *self = xmalloc(sizeof(Vector));

  self->data = xmalloc(sizeof(void *) * size);
  self->len = 0;
  self->reserved = size;

  for(int i = 0; i < size; ++i) {
    self->data[i] = NULL;
  }

  return self;
}

void del_vector(Vector *self) {
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

void vec_set_at(Vector *vec, int at, void *d) {
  if(at >= vec->reserved) {
    vec->reserved = at * 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->reserved);
  }
  vec->data[at] = d;
}

void vec_extend(Vector *vec, int size) {
  vec->data = realloc(vec->data, sizeof(void *) * size);
  vec->reserved = size;
}

void *vec_last(Vector *self) { return self->data[self->len - 1]; }

int get_digit(int num) {
  char buf[100];

  return sprintf(buf, "%d", num);
}

char *read_file(const char *path) {
  FILE *src_file = fopen(path, "r");
  if(!src_file) {
    return NULL;
  }

  fseek(src_file, 0, SEEK_END);
  size_t fsize = ftell(src_file);
  fseek(src_file, 0, SEEK_SET);

  char *src = calloc(1, fsize + 1);

  if(fread(src, 1, fsize, src_file) < fsize) {
    error("Error reading file");
  }

  fclose(src_file);

  return src;
}
