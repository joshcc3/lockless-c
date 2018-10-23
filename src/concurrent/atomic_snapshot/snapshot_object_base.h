#ifndef SNAP_BASE_H
#define SNAP_BASE_H

#include "concurrent/atomic_snapshot/snapshot_object.h"

void init_snapshot(int, const snapshot **);
void init_snapshot_from_existing(int, proc_local *, const snapshot **);

#endif
