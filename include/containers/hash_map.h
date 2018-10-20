#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stdlib.h>
#include "hash_set.h"


struct hash_map_element {
    const void * key;
    const void * value;
};

/*
  struct equals_typeclass {
      bool (*equal)(void const *, void const *);
      bool (*not_equal)(void const *, void const *);
  };
  struct obj_typeclass {
      hash_t (*hash)(void const *);
  };
*/

// A hash map consists of a hash set of a pair of (key, value) where equality between the elements is based on the key.
typedef struct hash_map {
    struct hash_set * wrapped;
    const struct equals_typeclass * e_t;
    const struct obj_typeclass * o_t;
} hash_map;

/*

  struct equals_typeclass {
      bool (*equal)(void const *, void const *);
      bool (*not_equal)(void const *, void const *);
  };
  struct obj_typeclass {
      hash_t (*hash)(void const *);
  };
*/

void init_hash_map(struct hash_map ** const, size_t, struct equals_typeclass const*, struct obj_typeclass const*);

err_t deinit_hash_map(struct hash_map ** const);

int hash_map_size(const struct hash_map *);

bool hash_map_contains_key(const struct hash_map *, const void *);

bool hash_map_get(const struct hash_map *, const void*, const void **);

void hash_map_put(struct hash_map **, const void*, const void *);

void hash_map_delete(struct hash_map **, const void *);

#endif
