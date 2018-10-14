#include <assert.h>
#include "snapshot_object.h"
#include <stdbool.h>

typedef log_t log_entry_t*

typedef log_entry_t *{
  int seq_no;
  int count;
} log_entry_t;

typedef struct atomic_closure {
  void (*func)(void*);
  void* args;
} atomic_closure;

void atomic(atomic_closure closure)
{
  return closure.func(closure.args);
}

typedef struct U_Args {
  proc_local* ref;
  snapshot new_snap;
  int val;
} U_Args;

void update_func(void* args_)
{
  U_Args args = *(U_Args*)args_;
  args.ref->val = args.val;
  args.ref->seq++;
  args.ref.global_state = args.new_snap
}

void collect(atomic_object ao, procid_t pid, proc_local** s)
{

}

bool snapshot_differs(int n, snapshot previous, snapshot current)
{
  protected seq_t* p_seqs = previous.seqs;
  protected seq_t* c_seqs = current.seqs;
  for(int i = 0; i < n; i++)
    if(p_seqs[i] != c_seqs[i]) return true;

  return false;
}

int update_log(log_t log, int i, int seq_t)
{
  if(log[i] != new_seq)
    {
      log[i].count++;
      log[i].seq_num = new_seq;
    }
  return log[i];
}

bool update_log_and_three_distinct_seqs(int n, log_t log, proc_local* c, snapshot *result)
{
  for(int i = 0; i < n; i++)
    {
      
      if(update_log(log, i, c[i].seq) >= 2)
	{
	  *result = *c.snap_base;
	  return true;
	}      
    }
  return false;
}

void ao_snap(atomic_object ao, procid_t pid, snapshot* snap)
{

  log_t log = (log_t)malloc(sizeof(log_entry_t) * ao.num_procs);
  assert(log);

  proc_local *previous;
  proc_local *current;
  collect(&previous);
  collect(&current);
  update_log_and_three_distinct_seqs(ao.num_procs, log, previous);
  update_log_and_three_distinct_seqs(ao.num_procs, log, current);  
  for(int i = 0; i < iteration_limit; i++) {
    previous = current;
    collect(&current);

    if(snapshot_differs(ao.num_procs, previous, current))
    {
      if(update_log_and_three_distinct_seqs(ao.num_procs, log, current))
	{
	  *snap = current;
	  return;
	}
    }
    else
      {
	// double collection - there was a valid snapshot
	*snap = current;
	return;
      }
  }
  assert(false);
}


void ao_update(atomic_object ao, procid_t pid, int val)
{
  snapshot snap;
  ao_snap(pid, &snap);
  U_Args args = (U_Args){ .ref = shared + pid, .new_snap = snap, .val = val };
  return atomic((atomic_closure) { .func = update_func, .args = (void*)&update_args });
}


void init_ao(int n)
{
  num_procs = n;
  shared = (proc_local*)malloc(sizeof(proc_local)*n);
  assert(shared);
}


