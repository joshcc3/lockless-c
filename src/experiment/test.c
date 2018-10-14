#include <stdio.h>

#define ITERATIONS 100000000
__int128_t shared;

void* checker(void* null)
{
  for(int i = 0; i < ITERATIONS; i++)
    {
      
    }
}

void* worker(void* arg)
{
  //  struct timespec sleep_time;
  int *toggleBits = (int*)arg;
  
  __int128_t one = 1;
  __int128_t b1 = one << toggleBits[0];
  __int128_t b2 = one << toggleBits[1];
  __int128_t vals[4] = {b1, b2, b1 + b2, 0};

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
  

  
}
