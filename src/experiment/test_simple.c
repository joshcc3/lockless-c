#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

/*
what happens to a thread when the parent dies:
  parent is the main function - the child is killed - there can be unintended effects (for example I think the pc of the child thread got overwritten and it reverted to a previous instruction?)
  parent is not the main function (and we pthread_exit in the main function) - the child is not killed and is preserved.

  Does calling pthread_exit in the parent thread wait for the child thread? (not main)  - yes it does
  Does calling pthread_exit in the parent thread wait for the grand child thread? (not main) - yes all children in the thread group are waited for if you call pthread_exit somewhere in the hierarchy

  Inference, it's only the exit syscall that actually clears stuff up.


if a thread joins on a non-deceased thread it does capture the return value.
if a thread joins on a deceased thread does is still capture its value? Yes, the thread still captures the value of the deceased thread.
Can a thread from another branch of the tree join on the og thread?

What is the de
When a message is broadcast, do the threads acquire 

 */




void* grand_child_thread(void *null_arg)
{
  printf("Grand Child thread about to sleep.\n");
  sleep(2);
  printf("Grand Child about to exit.\n");
}
void* child_thread(void *null_arg)
{
  pthread_t grand_t;
  pthread_create(&grand_t, NULL, grand_child_thread, NULL);
  printf("Child Exiting!\n");
}

void* parent_thread(void * null_arg)
{
  pthread_t child_t;
  pthread_create(&child_t, NULL, child_thread, NULL);
  printf("Parent thread exiting!\n");
}


void* test_join(void* value) {
  int *return_val = (int*)malloc(sizeof(int));
  *return_val = 21;

  printf("test_join exiting\n");
  pthread_exit((void*)return_val);
}

void* branch1(void *return_val)
{
  printf("Branch1 sleeping for 5 seconds\n");
  sleep(5);
  *(int*)return_val = 10;
  printf("Branch1 exiting\n");
  return return_val;
}

void* branch2(void *thread_to_wait)
{
  int *result;
  pthread_t *branch1 = (pthread_t*)thread_to_wait;
  printf("Branch2 joining branch1\n");
  pthread_join(*branch1, (void**)&result);
  printf("Branch2 reports that the return value of branch1 is %d\n", *result);
  assert(*result == 10);
  return NULL;
}

const int NUM_THREADS = 5;
typedef struct monitor {
  pthread_cond_t *condition_var;
  pthread_mutex_t *lock;
} monitor;

typedef struct waiting_thread {
  monitor ready_monitor;
  int *thread_ready_count;
} waiting_thread;

void* waiter_function(void *args_)
{
  pthread_t thread_id = pthread_self();
  waiting_thread *args = (struct waiting_thread*)args_;
  int sleep_time = (int)(rand()/(float)RAND_MAX * 10);
  printf("TID %x: Sleeping for %d\n", thread_id, sleep_time);
  
  sleep(sleep_time);

  // guarentee
  // 1. The broadcast signal is only sent when all threads have woken up

  printf("TID %x: About to enter the mutex to increment thread ready count\n", thread_id);
  pthread_mutex_lock(args->ready_monitor.lock);
  *args->thread_ready_count += 1; 
  if(*args->thread_ready_count == NUM_THREADS)
  {
    printf("TID %x: All %d have started, sending a broadcast signal\n", thread_id, *args->thread_ready_count);
    pthread_cond_broadcast(args->ready_monitor.condition_var);
  }
  else
  {
    printf("TID %x: Waiting for all threads to be ready, currently only %d have started\n", thread_id, *args->thread_ready_count);
    pthread_cond_wait(args->ready_monitor.condition_var, args->ready_monitor.lock);
    printf("TID %x: just received a signal, woken up and am plowing on\n", thread_id);
  }; 
  pthread_mutex_unlock(args->ready_monitor.lock);

}


void init_monitor(monitor *monitor)
{
  monitor->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  monitor->condition_var = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

  pthread_cond_init(monitor->condition_var, NULL);
  pthread_mutex_init(monitor->lock, NULL);

}

int main()
{



  // test order of wakeups on condition signal
  int thread_ready_count = 0;

  monitor ready_monitor;
  init_monitor(&ready_monitor);

  struct waiting_thread ts;
  ts.ready_monitor = ready_monitor;
  ts.thread_ready_count = &thread_ready_count;

  pthread_t tids[NUM_THREADS];


  printf("MAIN: Spawning the threads\n");
  for(int i = 0; i < NUM_THREADS; i++) 
  {
    pthread_t *t = tids + i;
    pthread_create(t, NULL, waiter_function, (void*)&ts);
  }
  printf("MAIN: Completed spawning %d threads\n", NUM_THREADS);
  pthread_mutex_lock(ts.ready_monitor.lock);
  while(*ts.thread_ready_count < NUM_THREADS)
  {
    printf("MAIN: Waiting for the all clear signal\n");
    pthread_cond_wait(ts.ready_monitor.condition_var, ts.ready_monitor.lock);
  }
  pthread_mutex_unlock(ts.ready_monitor.lock);

  printf("MAIN: Received the all clear, plowing ahead!\n");

  pthread_exit(NULL);

}


int test_pthread_join()
{
  pthread_t b1;
  pthread_t b2;

  int res;

  pthread_create(&b1, NULL, branch1, (void*)&res);
  pthread_create(&b2, NULL, branch2, &b1);

  pthread_exit(NULL);

}
// this prints start, start
