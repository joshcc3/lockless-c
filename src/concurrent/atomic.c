#include <stdbool.h>
#include "atomic.h"

void atomic_load(__int128_t* dest, __int128_t* src)
{
  __atomic_compare_exchange(dest, dest, src, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
void atomic_store(__int128_t* src, __int128_t* dest)
{
  __atomic_compare_exchange(src, dest, dest, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

