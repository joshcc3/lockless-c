#include <stdio.h>

#include "snapshot_object.h"


int main()
{
  atomic_object ao;
  init_ao(1, &ao);
  
  snapshot *snap1;
  ao_snap(ao, 0, &snap1);

  print_snap(snap1);

  ao_update(ao, 0, 123);

  snapshot *snap2;
  ao_snap(ao, 1, &snap2);
  print_snap(snap2);

  

}
