#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <concurrent/atomic.h>
#include <log.h>
#include <containers/hash_map.h>
#include <typeclasses.h>
#include <pthread.h>

#include "concurrent/atomic_snapshot/wait_free/unbounded_register/snapshot_object.h"

#define GET_LOCK(ao, lock) pthread_t *lock;		\
  hash_map_get(lock_pool, ao, (const void**)&lock);

#define WITH_OBJECT(ao, f, extra)			\
  {							\
    pthread_mutex_t *lock;					\
    hash_map_get(lock_pool, ao, (const void**)&lock);		\
    pthread_mutex_lock(lock);				\
    f(ao, extra);					\
    pthread_mutex_unlock(lock);				\
  }  

static hash_map *lock_pool;

struct update_action_args {
  procid_t pid;
  value v;
};

static void update_closure(atomic_object *ao, void *args_)
{
  struct update_action_args *args = (struct update_action_args*)args_;
  ao->shared[args->pid].val = args->v;
  ao->shared[args->pid].seq++;
}

static void ao_update(atomic_object *ao, procid_t pid, value v)
{
  log_info("START UPDATE\0");
  struct update_action_args args;
  args.pid = pid;
  args.v = v;
  WITH_OBJECT(ao, update_closure, (void*)&args)
  log_info("END UPDATE\0");    
}

struct snap_args {
  procid_t pid;
  const snapshot **snap;
};

static void snap_closure(atomic_object *ao, void *args_)
{
  struct snap_args* args = (struct snap_args*)args_;
  init_snapshot(ao->num_procs, args->snap);
  snapshot *snap = *(args->snap);
  value *vs = (value*)malloc(sizeof(value)*ao->num_procs);
  seq_t *ss = (seq_t*)malloc(sizeof(seq_t)*ao->num_procs);
  for(int i = 0; i < ao->num_procs; i++)
    {
      vs[i] = ao->shared[i].val;
      ss[i] = ao->shared[i].seq;
    }
  snap->values = vs;
  snap->seqs = ss;
}

void ao_snap(struct atomic_object* ao, procid_t pid, const snapshot** snap)
{
  log_info("START SNAP\0");
  struct snap_args args;
  args.snap = snap;
  args.pid = pid;
  WITH_OBJECT(ao, snap_closure, (void*)&args)
  log_info("END SNAP\0");  
}

void deinit_locking_ao(atomic_object *ao)
{
  GET_LOCK(ao, lock)
  free(lock);
}

void init_locking_ao(const int n, atomic_object *ao)
{
  ptr_obj_typeclass_witness.hash(ao, NULL);
  init_hash_map(&lock_pool, 64, &ptr_equals_typeclass_witness, &ptr_obj_typeclass_witness);
  pthread_mutex_t *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex, NULL);
  hash_map_put(&lock_pool, (void*)ao, (void*)mutex);

  proc_local *procs;
  init_proc_local(n, &procs);
  ao->shared = procs;
  ao->num_procs = n;
  ao->update = ao_update;
  ao->snap = ao_snap;
}
