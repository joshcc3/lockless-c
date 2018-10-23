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
  
struct atomic_object {
  proc_local * shared;
  int num_procs;
  void (*update)(struct atomic_object*, procid_t, value);
  void (*snap)(struct atomic_object*, procid_t, const snapshot**);
};
typedef struct atomic_object atomic_object;


void init_ao(const int, atomic_object *);
//void deinit_ao(const int, atomic_object *);

void print_snap(int, const snapshot*);

#endif
