#ifndef SNAP_H
#define SNAP_H

typedef procid_t int;
typedef value int;
typedef seq_t uint;

typedef struct snapshot {
  value* values;
  seq_t* seqs;
} snapshot;

typedef struct proc_local {
  value val;
  seq_t seq;
  snapshot* global_state;
} proc_local;

typedef struct atomic_object {
  proc_local *shared;
  int num_procs;
} atomic_object;


void init_ao(int, atomic_object*);

void ao_update(atomic_object, procid_t, value);
void ao_snap(atomic_object, procid_t, snapshot*);

#endif SNAP_H
