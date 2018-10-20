#ifndef MMANAGER_H
#define MMANAGER_H

#define NEW(val_type, val) val_type *val = (val_type *)new(sizeof(val_type));

void* new(size_t);

void* new_calloc(int, size_t);

/* frees the memory allocated with the pointer and nulls out the pointer */
void delete(void **);

#endif
