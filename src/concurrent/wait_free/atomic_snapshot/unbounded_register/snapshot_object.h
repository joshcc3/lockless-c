#ifndef SNAP_H
#define SNAP_H

typedef int procid_t;
typedef int value;
typedef int seq_t;

typedef struct snapshot {
  const value* values;
  const seq_t* seqs;
} snapshot;

struct proc_local {
  value val;
  seq_t seq;
  const snapshot* snap_base;
} __attribute__ ((packed, aligned (128)));

typedef struct proc_local proc_local;

typedef struct atomic_object {
  proc_local * shared;
  int num_procs;
} atomic_object;


void init_ao(int, atomic_object*);

void ao_update(atomic_object, procid_t, value);
void ao_snap(atomic_object, procid_t, snapshot**);

void print_snap(int, snapshot*);

#endif
