#include <log.h>
#include <snapshot_object.h>

int single_threaded()
{
  atomic_object ao;
  init_locking_ao(1, &ao);
  snapshot *s1;
  ao->snap(ao, 0, &s1);
  ao->update(ao, 0, 1);
  ao->snap(ao, 0, &s1);
}

int main()
{
  single_threaded();
}
