#ifndef LLIST_H
#define LLIST_H

#include <stdbool.h>

#include "err.h"
#include "typeclasses.h"

struct node {
  const void *element;
  struct node *next;
};

struct linked_list {
  int size;
  struct node *head;
  struct node *tail;
};


struct linked_list * new_llist();

err_t deinit_llist(struct linked_list**);

int ll_size(struct linked_list const *);

bool ll_is_empty(struct linked_list* l);

err_t ll_insert_element(struct linked_list * const linked_list, const int n, const void * element);

err_t ll_delete_element(struct linked_list * const linked_list, const int n, const void ** result);

err_t ll_get_element(struct linked_list const * const, const int, const void **);

err_t ll_search(struct linked_list const *, void const *, struct equals_typeclass const *, void const **);

err_t ll_index_of(struct linked_list const *, void const *, struct equals_typeclass const *, int *);

err_t ll_map (struct linked_list const * const, err_t (*)(void**));


#endif
