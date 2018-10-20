#ifndef TYPECLASSES_H
#define TYPECLASSES_H

#include <stdbool.h>
typedef int hash_t;

struct equals_typeclass {
    bool (*equal)(void const *, void const *, void const *);
    bool (*not_equal)(void const *, void const *, void const *);
    const void * extra;
};
struct obj_typeclass {
    hash_t (*hash)(void const *, void const *);
    const void * extra;
};

const struct equals_typeclass ptr_equals_typeclass_witness;
const struct equals_typeclass int_ptr_equals_typeclass;
const struct obj_typeclass int_ptr_obj_typeclass;

#endif
