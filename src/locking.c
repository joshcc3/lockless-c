#include <log.h>
#include "concurrent/atomic_snapshot/locking/snapshot_object.h"
#include "concurrent/atomic_snapshot/snapshot_object.h"


void single_threaded()
{
  atomic_object ao;
  init_locking_ao(1, &ao);
  const snapshot *s1;
  ao.snap(&ao, 0, &s1);
  ao.update(&ao, 0, 1);
  ao.snap(&ao, 0, &s1);
}

int main()
{
  single_threaded();
}
