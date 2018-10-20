#include <concurrent/atomic.h>

int main()
{
  __int128_t a;
  atomic_load(&a, &a);
}
