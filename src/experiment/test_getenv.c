#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  char *run_name = getenv("RUN_NAME");
  assert(run_name);
  printf("%s\n", run_name);
  printf("%s\n", getenv("HOME"));
  printf("%s\n", getenv("HOME"));  
}
