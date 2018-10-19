#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "snapshot_object.h"
#include "concurrent/atomic.h"

#define ITERATION_LIMIT(x) x

// TODO: add const correctness everywhere

typedef log_t log_entry_t*;

typedef struct log_entry_t {
  int seq_no;
  int count;
} log_entry_t;

typedef uint128_t byte[128];

// use restrict qualified to vectorize the copying of proc local states
void collect(atomic_object ao, procid_t pid, proc_local** s)
{
  *s = (proc_local*)malloc(sizeof(proc_local) * ao.num_procs);
  for(int i = 0; i < ao.num_procs; i++)
  {
    // TODO should be atomic
    atomic_load((__int128_t*)(ao.shared + i), (__int128_t*)s[i]);
  }
}

bool snapshot_differs(int n, snapshot *previous, snapshot *current)
{
  protected seq_t* p_seqs = previous.seqs;
  protected seq_t* c_seqs = current.seqs;
  for(int i = 0; i < n; i++) if(p_seqs[i] != c_seqs[i]) return true;

  return false;
}

int update_log(log_t log, int i, int new_seq)
{
  if(log[i] != new_seq)
  {
      log[i].count++;
      log[i].seq_num = new_seq;
  }
  return log[i];
}

// This checks for 3 distinct values from a process and if it find it sets result
bool update_and_check(int n, log_t log, proc_local* c, snapshot **result)
{
  assert(log && c && result);
  for(int i = 0; i < n; i++)
  {
    log_entry_t old_entry = log[i];
    if(update_log(log, i, c[i].seq) >= 2)
    {
      *result = c[i].snap_base;
      assert(old_entry.seq < c[i].seq || old_entry.count == 2);
      return true;
    }
    assert(old_entry.seq == c[i].seq || old_entry.count == 1);
  }
  for(int i = 0; i < n; i++) assert(log[i].count <= 1);
  return false;
}


void ao_snap(atomic_object ao, procid_t pid, snapshot** snap)
{
  // Pre: valid args
  // Post: In the new snapshot, seq numbers must either have increased or values are unchanged

  assert(ao.num_procs >= 1
	 && pid >= 0
	 && pid < ao.num_procs
	 && snap);
  for(int i = 0; i < ao.num_procs; i++) assert(ao.shared[i].seq >= 0);

  snapshot *old_snap = ao.shared[pid].snap_base;

  log_t log = (log_t)calloc(ao.num_procs, sizeof(log_entry_t));
  assert(log);

  proc_local *previous;
  proc_local *current;
  collect(ao, pid, &previous);
  collect(ao, pid, &current);
  update_and_check(ao.num_procs, log, previous);
  update_and_check(ao.num_procs, log, current); 
  for(int i = 0; i < ITERATION_LIMIT(n); i++) {
    if(snapshot_differs(ao.num_procs, previous, current))
    {
      if(update_and_check(ao.num_procs, log, current, snap))
	{
	  for(int i = 0; i < n; i++)
	    assert(*snap->seqs[i] > old_snap->seqs[i] || (*snap->seqs[i] == old_snap->seqs[i] && *snap->values[i] == old_snap->values[i]));
	  return;
	}
    }
    else
      {
	// double collection - there was a valid snapshot
	init_snapshot(current, snap);
	assert(*snap->seqs[i] > old_snap->seqs[i] || (*snap->seqs[i] == old_snap->seqs[i] && *snap->values[i] == old_snap->values[i]));	
	return;
      }
    previous = current;
    collect(ao, pid, &current);
  }
  assert(false);
}


void ao_update(atomic_object ao, procid_t pid, int val)
{
  assert(ao.num_procs >= 1
	 && pid >= 0
	 && pid < ao.num_procs
	 && ao.shared[0].seq >= 0);
  int prev_seq = ao.shared[pid].seq;
  int prev_val = ao.shared[pid].val;
  const snapshot *snap;
  ao_snap(pid, &snap);

  __int128_t new_proc_local;
  int* tmp = (int*)&new_proc_local;
  tmp[0] = val;
  tmp[1] = prev_seq + 1;
  (const snapshot*)(tmp + 2) = snap;

  atomic_store((__int128_t*)(ao.shared + pid), &new_proc_local);

  assert(ao.shared[pid].val == val && ao.shared[pid].seq == prev_seq + 1);
}

void print_snap(int n, snapshot* snap)
{
  for(int i = 0; i < n - 1; i++) printf("(%d, %d), ", snap->values[i], snap->seqs[i]);
  printf("(%d, %d)\n", snap->values[n-1], snap->seqs[n - 1]);
}

void init_snapshot(int n, snapshot **res)
{
  *res = (snapshot*)malloc(sizeof(snapshot)*n);
  value *values = (value*)calloc(n, sizeof(value));
  seq_t *seqs = (seq_t*)malloc(n, sizeof(seq_t));
  *res->values = values;
  *res->seqs = seqs;
}

void init_snapshot(int n, proc_local *snapped_state, snapshot **res)
{
  
  init_snapshot(n, res);
  assert(*res);

  snapshot new_snap = **res;

  for(int i = 0; i < n; i++)
    {
      new_snap.values[i] = snapped_state[i].val;
      new_snap.seqs[i] = snapped_state[i].seq;      
    }
  assert(*res);
}


static void init_proc_local(int n, proc_local **procs)
{
  *procs = (proc_local*)malloc(sizeof(proc_local)*n);
  assert(shared);

  snapshot *snap;
  init_snapshot(ao->num_procs, &snap);

  for(int i = 0; i < n; i++) proc_local[i] = (proc_local){ .val = 0, .seq = 0, .snap_base = snap };

  assert(*procs);

}

void init_ao(const int n, atomic_object *ao)
{
  assert(n >= 1);

  num_procs = n;

  proc_local *procs;
  init_proc_local(ao->num_procs, &procs);
  // check that procs was inited and that the value in the snap that n-1 has for n-1 matches what it stores locally
  assert(procs && procs[n-1].seq == 0 && procs[n-1].val == procs[n-1].snap_base[n-1].values[n-1]);

  atomic_object.shared = procs;
  atomic_object.num_procs = n;

}
