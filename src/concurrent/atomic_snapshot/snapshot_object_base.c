#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "snapshot_object_base.h"
#include "concurrent/atomic.h"
#include <log.h>

void print_snap(int n, const snapshot* snap)
{
  char buffer[2048];
  for(int i = 0; i < n - 1; i++)
    {
      sprintf(buffer, "%s(%d, %d), ", buffer, snap->values[i], snap->seqs[i]);
    }
  sprintf(buffer, "%s(%d, %d)\0", buffer, snap->values[n-1], snap->seqs[n-1]);
  log_info(buffer);
}

void init_proc_local(int n, proc_local **procs)
{
  *procs = (proc_local*)malloc(sizeof(proc_local)*n);
  assert(*procs);

  const snapshot *snap;
  init_snapshot(n, &snap);

  for(int i = 0; i < n; i++) (*procs)[i] = (proc_local){ .val = 0, .seq = 0, .snap_base = snap };

  assert(*procs);

}

void init_snapshot(int n, const snapshot **res)
{
  snapshot *tmp = (snapshot*)malloc(sizeof(snapshot)*n);
  const value *values = (value*)calloc(n, sizeof(value));
  const seq_t *seqs = (seq_t*)calloc(n, sizeof(seq_t));
  tmp->values = values;
  tmp->seqs = seqs;
  *res = tmp;
}

void init_snapshot_from_existing(int n, proc_local *snapped_state, const snapshot **res)
{
  
  init_snapshot(n, res);
  assert(*res);

  snapshot *new_snap = (snapshot*)*res;
  
  value* tmp_values = (value*)malloc(n*sizeof(value));
  seq_t* tmp_seqs = (seq_t*)malloc(n*sizeof(seq_t));

  for(int i = 0; i < n; i++)
    {
      proc_local tmp;
      atomic_load((__int128_t*)&tmp, (__int128_t*)(snapped_state + i));
      tmp_values[i] = tmp.val;
      tmp_seqs[i] = tmp.seq;
    }
  new_snap->values = tmp_values;
  new_snap->seqs = tmp_seqs;
  assert(*res);
}
