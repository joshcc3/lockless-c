#include <stdlib.h>
#include <stdio.h>

int main(int argc, char**argv)
{
  if(argc < 2) { perror("Not enough arguments\n"); return 1; }
  for(int i = 0; i < 100; i++) printf("%d: %s\n", i, argv[i]);
}
