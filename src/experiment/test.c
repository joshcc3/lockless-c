#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

// The monitor closure
typedef struct monitor {
  pthread_mutex_t lock;
  pthread_cond_t condition;
  bool (*condition_test)(void*);
  void*extra;
} monitor_t;



#define ITERATIONS 1000000

#define NUM_THREADS 10000

__int128_t shared;

void monitor_notify(monitor_t *m) { pthread_cond_signal(&m->condition); }

void monitor_broadcast(monitor_t *m) { pthread_cond_broadcast(&m->condition); }

void monitor_wait(monitor_t *m)
{
  assert(m);
  pthread_mutex_lock(&m->lock);
  while(!m->condition_test(m->extra)) pthread_cond_signal(&m->condition);
  assert(m->condition_test(m->extra));
  pthread_mutex_unlock(&(m->lock));
}

void monitor_deinit(monitor_t *m)
{
  assert(m);
  pthread_mutex_destroy(&(m->lock));
  //pthread_cond_destory(&(m->condition));
  free(m);
}

void monitor_init(bool (*c)(void*), monitor_t **m, void*extra) 
{
  assert(m && c);
  *m = (monitor_t*)malloc(sizeof(monitor_t));
  
  pthread_mutex_init(&((*m)->lock), NULL);
  pthread_cond_init(&((*m)->condition), NULL);
  (*m)->condition_test = c;
  (*m)->extra = extra;

  assert(*m != NULL);
}



int err_count = 0;

void log_err(__int128_t tmp, int iter)
{
  err_count++;
  unsigned long long *tmp_ = (unsigned long long *)&tmp;
  printf("More than 2 bits were set in 0x%llu%llu on iteration %d\n", tmp_[0], tmp_[1], iter);
}

void* checker(void* all_done)
{
  
  printf("CHECKER: Started\n");

  for(int i = 0; i < !*(bool*)all_done; i++)
    {
      __int128_t tmp = shared;
      if(__builtin_popcount(tmp) > 2)
	{
	  log_err(tmp, i);
	}
    }
}




typedef struct worker_args {
  int toggleBits[2];
  monitor_t monitor;
}worker_args;
void* worker(void* arg)
{
  //  struct timespec sleep_time;
  int *toggleBits = (int*)arg;
  //printf("[%d, %d] Thread created\n", toggleBits[0], toggleBits[1]);
  __int128_t one = 1;
  __int128_t b1 = one << toggleBits[0];
  __int128_t b2 = one << toggleBits[1];
  __int128_t vals[4] = {b1, b2, b1 + b2, 0};

  //printf("Thread-%d: Starting thread\n", (int)(sqrt(toggleBits[0]*128 + toggleBits[1])));

  for(int i = 0; i < ITERATIONS; i++)
    {
      shared = vals[i%4];
    }
}


int main()
{
  // Test whether reads and writes to a 128 bit value are atomic or not
  /*
    Spawn 500 threads each of which writes all 0s except to 2 bits which it toggles.
    A check thread continusouly reads from the value and reports if it ever sees
    a state where more than 1 bit is 1.
   */
  bool all_done = false;
  pthread_t checker_t;
  pthread_create(&checker_t, NULL, checker, (void*)&all_done);
  printf("MAIN: Started my checker thread\n");
  int args[NUM_THREADS][2];
  pthread_t tids[NUM_THREADS];
  for(int i = 0; i < NUM_THREADS; i++)
    {
      args[i][0] = ((i*i)%(128*128))/128;
      args[i][1] = ((i*i)%(128*128))%128;
      //printf("MAIN: Created child %d\n", i);
      pthread_create(tids + i, NULL, worker, (void*)args[i]);
    }
  for(int i = 0; i < NUM_THREADS; i++) pthread_join(tids[i], NULL);
  all_done = true;
  pthread_exit(NULL);

}
