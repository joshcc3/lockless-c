#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "concurrent/atomic_snapshot/wait_free/unbounded_register/snapshot_object.h"
#include "concurrent/atomic.h"
#include <log.h>

#define ITERATION_LIMIT(x) x

// TODO: add const correctness everywhere

typedef struct log_entry_t {
  int seq_no;
  int count;
} log_entry_t;

typedef log_entry_t* log_t;

// use restrict qualified to vectorize the copying of proc local states
static void collect(atomic_object ao, procid_t pid, proc_local** s)
{
  log_info("COLLECT BEGIN\0");
  *s = (proc_local*)malloc(sizeof(proc_local) * ao.num_procs);
  for(int i = 0; i < ao.num_procs; i++) atomic_load((__int128_t*)(*s + i), (__int128_t*)(ao.shared + i));
  log_info("COLLECT END\0");
}

static bool proc_state_differs(int n, proc_local* previous, proc_local* current)
{
  for(int i = 0; i < n; i++) if(previous[i].seq != current[i].seq) return true;

  return false;
}

static int update_log(log_t log, int i, int new_seq)
{
  if(log[i].seq_no != new_seq)
  {
      log[i].count++;
      log[i].seq_no = new_seq;
  }
  return log[i].count;
}

// This checks for 3 distinct values from a process and if it find it sets result
static bool update_and_check(int n, log_t log, const proc_local* c, const snapshot **result)
{
  assert(log && c && result);
  for(int i = 0; i < n; i++)
  {
    log_entry_t old_entry = log[i];
    if(update_log(log, i, c[i].seq) == 3)
    {
      *result = c[i].snap_base;
      assert(old_entry.seq_no < c[i].seq || old_entry.count == 3);
      return true;
    }
    assert(old_entry.seq_no == c[i].seq || log[i].count < 3);
  }
  for(int i = 0; i < n; i++) assert(log[i].count <= 2);
  return false;
}

static void ao_snap(atomic_object *ao, procid_t pid, const snapshot** snap)
{
  // Pre: valid args
  // Post: In the new snapshot, seq numbers must either have increased or values are unchanged

  log_info("START SNAP\0");
  assert(ao->num_procs >= 1
	 && pid >= 0
	 && pid < ao->num_procs
	 && snap);
  for(int i = 0; i < ao->num_procs; i++) assert(ao->shared[i].seq >= 0);

  const snapshot *old_snap = ao->shared[pid].snap_base;

  log_t log = (log_t)calloc(ao->num_procs, sizeof(log_entry_t));
  assert(log);

  proc_local *previous;
  proc_local *current;
  collect(*ao, pid, &previous);
  collect(*ao, pid, &current);
  update_and_check(ao->num_procs, log, previous, snap);
  update_and_check(ao->num_procs, log, current, snap); 
  for(int i = 0; i < ITERATION_LIMIT(ao->num_procs); i++) {
    if(proc_state_differs(ao->num_procs, previous, current))
    {
      log_info("DOUBLE COLLECT FAIL - %d\0", i + 1);
      if(update_and_check(ao->num_procs, log, current, snap))
	{
	  log_info("CASE B - %d\0", i+1);
	  for(int i = 0; i < ao->num_procs; i++) assert((*snap)->seqs[i] > old_snap->seqs[i] || ((*snap)->seqs[i] == old_snap->seqs[i] && (*snap)->values[i] == old_snap->values[i]));
	  break;
	}
    }
    else
      {
	// double collection - there was a valid snapshot
	log_info("CASE A - %d\0", i + 1);
	init_snapshot_from_existing(ao->num_procs, current, snap);
	assert((*snap)->seqs[i] > old_snap->seqs[i] || ((*snap)->seqs[i] == old_snap->seqs[i] && (*snap)->values[i] == old_snap->values[i]));
	break;
      }
    free(previous);
    previous = current;
    collect(*ao, pid, &current);

  }
  log_info("END SNAP\0");
}


static void ao_update(atomic_object *ao, procid_t pid, int val)
{
  log_info("START UPDATE\0");
  assert(ao->num_procs >= 1
	 && pid >= 0
	 && pid < ao->num_procs
	 && ao->shared[0].seq >= 0);
  int prev_seq = ao->shared[pid].seq;
 
  const snapshot *snap;
  ao_snap(ao, pid, &snap);

  __int128_t new_proc_local;
  int* tmp = (int*)&new_proc_local;
  tmp[0] = val;
  tmp[1] = prev_seq + 1;
  *(const snapshot**)(tmp + 2) = snap;

  atomic_store(&new_proc_local, (__int128_t*)(ao->shared + pid));

  assert(ao->shared[pid].val == val && ao->shared[pid].seq == prev_seq + 1);
  log_info("END UPDATE\0");
}

void init_wait_free_ao(const int n, atomic_object *ao)
{
  assert(n >= 1);

  ao->num_procs = n;

  proc_local *procs;
  init_proc_local(ao->num_procs, &procs);
  ao->shared = procs;
  ao->update = ao_update;
  ao->snap = ao_snap;

  // check that procs was inited and that the value in the snap that n-1 has for n-1 matches what it stores locally
  assert(procs && procs[n-1].seq == 0 && procs[n-1].val == procs[n-1].snap_base->values[n-1]);


}
