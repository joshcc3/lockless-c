#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <data/string_.h>
#include <sys/syscall.h>
#include <log.h>

#define gettid() syscall(SYS_gettid)

void print_snap(int n, const int *a)
{
  char buffer[2048];
  memset(buffer, 0, 1);
  for(int i = 0; i < n - 1; i++)
    {
      sprintf(buffer, "%s(%d, %d), ", buffer, *(a++), *(a++));
    }
  sprintf(buffer, "%s(%d, %d)", buffer,  *(a++), *(a++));
  log_info(buffer);
}

void* worker(void* arg)
{
  for(int j = 0; j < 1000; j++)
    {
      int a[10];
      for(int i = 0; i < 10; i++) a[i] = i;
      print_snap(5, a);
    }
}


int main()
{
  pthread_t pids[100];
  for(int i = 0;i < 100; i++)
    {
      pthread_create(pids + i, NULL, worker, NULL);
    }
  pthread_exit(NULL);
}
