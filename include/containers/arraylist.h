#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdlib.h>

#include "err.h"
#include "resizable_array.h"

struct arr_list {
    struct resizable_array * wrapped;
};

err_t new_arrlist(size_t, struct arr_list**);

err_t deinit_arrlist(struct arr_list **);

int arrl_size(const struct arr_list *const);

int arrl_capacity(const struct arr_list *const);

int arrl_is_empty(const struct arr_list *const);

err_t arrl_get_element(struct arr_list const * const, const int, const void **);

err_t arrl_append(struct arr_list *, const void *);

err_t arrl_pop_back(struct arr_list *, const void **);

#endif
