#ifndef INT_HASHSET_H
#define INT_HASHSET_H

#include <stdlib.h>
#include <stdbool.h>

#include "llist.h"
#include "arraylist.h"
#include "resizable_array.h"
#include "err.h"
#include "typeclasses.h"


struct int_hash_set {
    // table of linked-lists
    struct resizable_array* hash_table;

    size_t size;

};


err_t hset_init(struct hash_set **, size_t);

err_t hset_deinit(struct hash_set **);

int hset_size(struct hash_set const *);

bool hset_is_empty(struct hash_set *);

int hset_contains(struct hash_set *, const void *, struct equals_typeclass const*, struct obj_typeclass  const*);

err_t hset_insert_element(struct hash_set **, const void *, struct equals_typeclass const*, struct obj_typeclass  const*);

err_t hset_delete_element(struct hash_set **, const void *, const void **, struct equals_typeclass const* e_t,
                          struct obj_typeclass const* o_t);

err_t hset_to_arr_list(struct hash_set const *, struct arr_list **);

err_t hset_get_element(struct hash_set *inp, const void *element, struct equals_typeclass const * e_t,
                       struct obj_typeclass const * o_t, const void **result);

#endif
