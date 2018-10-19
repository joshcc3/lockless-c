#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "snapshot_object.h"
#include <time.h>

int rand_in_range(int mx) { return (int)((double)rand()/RAND_MAX * mx); }


void single_threaded_test()
{
  int num_procs = 1;
  atomic_object ao;
  init_ao(num_procs, &ao);
  
  const snapshot *snap1;
  ao_snap(ao, 0, &snap1);

  print_snap(num_procs, snap1);

  ao_update(ao, 0, 123);

  const snapshot *snap2;
  ao_snap(ao, 0, &snap2);
  print_snap(num_procs, snap2);


  ao_update(ao, 0, 321);

  const snapshot *snap3;
  ao_snap(ao, 0, &snap3);
  print_snap(num_procs, snap3);
  
}

void single_threaded_multiple_processes()
{
  int num_procs = 5;
  atomic_object ao;
  init_ao(num_procs, &ao);
  ao_update(ao, 0, 100);
  ao_update(ao, 1, 200);

  const snapshot *s1;
  ao_snap(ao, 1, &s1);
  print_snap(5, s1);
  print_snap(5, s1);
  print_snap(5, s1);    
  
  ao_update(ao, 2, 300);
  ao_update(ao, 3, 400);
  ao_update(ao, 0, 101);
  
  const snapshot *s2;
  ao_snap(ao, 2, &s2);
  print_snap(5, s2);
  
  ao_update(ao, 4, 500);
  ao_update(ao, 1, 201);
  ao_update(ao, 2, 301);
  
  const snapshot *s3;
  ao_snap(ao, 4, &s3);
  print_snap(5, s3);

}

struct worker_args {
  atomic_object *obj;
  int pid;
  int iterations;
  int checkpoint_count;
  int num;
};

void* worker(void* args_)
{
  struct worker_args* args = (struct worker_args*)args_;
  
  atomic_object ao = *(args->obj);
  int pid = args->pid;

  const snapshot *initial_snap;
  ao_snap(ao, pid, &initial_snap);
  int acc = initial_snap->values[pid];

  for(int i = 0; i < args->iterations; i++)
    {
      acc += rand_in_range(100);
      ao_update(*(args->obj), pid, acc);

      if(i%args->checkpoint_count == 0)
	{
	  const snapshot *initial;
	  ao_snap(ao, pid, &initial);
	  int total_count = 0;
	  for(int j = 0; j < ao.num_procs; j++) total_count += initial->values[j];
	  printf("Thread-%d: (iteration %d, tot_sum %d)\n", pid, i, total_count);
	}
      int rand_sleep = rand_in_range(200);
      const struct timespec tm = (struct timespec){ .tv_sec = 0, .tv_nsec = rand_sleep };
      nanosleep(&tm, NULL);
    }
    return NULL;
}

void multi_threaded_app()
{
  int num = 10;
  atomic_object ao;
  init_ao(num, &ao);
  pthread_t pids[num];
  for(int i = 0; i < num; i++)
    {
      struct worker_args *args = (struct worker_args*)malloc(sizeof(struct worker_args));
      *args = (struct worker_args){ .obj = &ao, .pid = i, .iterations = 10000, .checkpoint_count = 100, .num = num };
      pthread_create(pids + i, NULL, worker, args);
    }

  pthread_exit(NULL);
  
}

int main()
{
  multi_threaded_app();
}
