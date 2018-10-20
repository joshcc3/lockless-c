#ifndef RESIZABLE_ARRAY_H
#define RESIZABLE_ARRAY_H

#include <stdlib.h>
#include "err.h"

#define MIN_SIZE 16

struct resizable_array;

struct resizable_array {
    size_t capacity;
    size_t size;
    const void* *array;
};


err_t new_resizable_array(size_t, struct resizable_array**);

err_t deinit_resizable_array(struct resizable_array **);

int resizable_array_size(const struct resizable_array *const);

int resizable_array_capacity(const struct resizable_array *const);

int resizable_array_is_empty(const struct resizable_array *const);

err_t resizable_array_get_element(struct resizable_array const * const, const int, const void **);

err_t resizable_array_set_element(struct resizable_array const *, int, const void *);

err_t resizable_array_append(struct resizable_array * const, const void *);

err_t resizable_array_pop_back(struct resizable_array *const, const void **);

#endif
