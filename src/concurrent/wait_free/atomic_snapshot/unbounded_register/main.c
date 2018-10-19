#include <stdio.h>

#include "snapshot_object.h"


int main()
{
  int num_procs = 1;
  atomic_object ao;
  init_ao(num_procs, &ao);
  
  const snapshot *snap1;
  ao_snap(ao, 0, &snap1);

  print_snap(num_procs, snap1);

  ao_update(ao, 0, 123);

  const snapshot *snap2;
  ao_snap(ao, 0, &snap2);
  print_snap(num_procs, snap2);


  ao_update(ao, 0, 321);

  const snapshot *snap3;
  ao_snap(ao, 0, &snap3);
  print_snap(num_procs, snap3);
  
  

}
