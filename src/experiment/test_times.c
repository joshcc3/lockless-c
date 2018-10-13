#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t cas_lock;


typedef struct lock_stats {
  int lock_attempts;
  int contended;
} lock_stats;

typedef struct lock_t {
  int state;
  struct lock_stats stats;
} lock_t;

struct timespec LOCK_SLEEP_INTERVAL = { .tv_sec = 0, .tv_nsec = 1000 };

pthread_mutex_t cas_lock;

bool compare_and_swap(lock_t *lock, int a, int b)
{
  pthread_mutex_lock(&cas_lock);
  if(lock->state != a) {
    pthread_mutex_unlock(&cas_lock);
    return 0;
  }
  lock->state = b;

  pthread_mutex_unlock(&cas_lock);

  return 1;

}


void lock_init(lock_t *lock)
{
  lock->state = 0;
}


void lock(lock_t *lock)
{
  // state transitions:
  /*
   * 0 - unlocked: switch to contended, retry on failure
   * 1 - contended but not locked: cas to 2, retry on failure
   * 2 - locked: sleep for some time and then retry
   */

  pthread_mutex_lock(&cas_lock);
  lock->stats.lock_attempts++;
  pthread_mutex_unlock(&cas_lock);
  while(1)
  {
    if(lock->state == 0)
    {
      compare_and_swap(lock, 0, 1);
    }
    else if(lock->state == 1)
    {
      bool result = compare_and_swap(lock, 1, 2);
      if(result) break;
    }
    else
    {
      pthread_mutex_lock(&cas_lock);
      lock->stats.contended++;
      pthread_mutex_unlock(&cas_lock);
      nanosleep(&LOCK_SLEEP_INTERVAL, NULL);
    }
  }

}

void unlock(lock_t *lock)
{
  if(lock->state == 2) lock->state = 0;
  else assert(false);
}


void pprint_lock_stats(lock_stats stats)
{
  printf("Lock Attempts: %d, Number of times Contended: %d, Lock Contention%: %f\n", stats.lock_attempts, stats.contended, ((double)stats.contended)/stats.lock_attempts*100.0d);
}

void pprint_tms(struct tms *tms)
{
  double clock_ticks_per_ns = (double)(sysconf(_SC_CLK_TCK))/1000000000.0D;
  double user_time = (double)tms->tms_utime/clock_ticks_per_ns/1000000;
  double sys_time = (double)tms->tms_stime/clock_ticks_per_ns/1000000;
  printf("RAW: Clock Ticks - User time: %li, System Time: %li\n", tms->tms_utime, tms->tms_stime);
  printf("User time: %f ms, System Time: %f ms\n", user_time, sys_time);
}

int main_1()
{

  struct timespec sleeptime;
  sleeptime.tv_sec = 1;
  sleeptime.tv_nsec = 0;

  nanosleep(&sleeptime, NULL);

  struct tms tms;
  times(&tms);

  pprint_tms(&tms);
}


lock_t global;
long large_sum = 0;

void* child_fun(void* null)
{
  for(int i = 0; i > -1000000; i--)
  {

    lock(&global);
    large_sum += i;
    unlock(&global);
    if(i % 10000 == 0) printf("CHILD: Progressed to %d\n", i);
  }
  
}

int main_test_lock_cont()
{

  struct tms tms;
  lock_init(&global);

  pthread_t child;
  pthread_create(&child, NULL, child_fun, NULL);

  for(int i = 0; i < 1000000; i++)
  {

    lock(&global);
    large_sum += i;
    unlock(&global);
    if(i % 10000 == 0) printf("MAIN: Progressed to %d\n", i);
  }

  pthread_join(child, NULL);
  printf("Sum result (0): %d\n", large_sum);
  pprint_lock_stats(global.stats);
  printf("Clock Ticks: %li \n", times(&tms));
  pprint_tms(&tms);
  pthread_exit(NULL);
}


int main()
{
  struct tms tms;

  printf("Clock ticks per second: %d\n", sysconf(_SC_CLK_TCK));


  long large_sum = 0;
  for(int i = 0; i < 1000 * 10000; i++)
    {
      large_sum += i;
    }

  times(&tms);
  pprint_tms(&tms);
}
