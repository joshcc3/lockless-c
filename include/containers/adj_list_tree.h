#ifndef ADJ_LIST_TREE
#define ADJ_LIST_TREE

#include "hash_map.h"

#define MAGIC_MULTIPLIER 2

struct adj_list_tree {
  // hash map from int to hash set of int
  int nodes;
  struct hash_map *wrapped;
  
};

err_t init_adj_list_tree(struct adj_list_tree **, int);
err_t deinit_adj_list_tree(struct adj_list_tree **);

bool adj_list_tree_add_edge(struct adj_list_tree *, int, int);
bool adj_list_tree_delete_edge(struct adj_list_tree *, int, int);
err_t adj_list_tree_get_edges(const struct adj_list_tree *, int, const struct linked_list **);
bool adj_list_tree_edge_exists(const struct adj_list_tree *, int, int);

#endif
