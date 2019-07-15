# Atomic Snapshots
A pure C implementation of wait free atomic snapshots on unbounded registers based on (the simplest algorithm) http://www.cs.yale.edu/homes/aspnes/pinewiki/AtomicSnapshots.html#The_Gang_of_Six_algorithm


## Wait free
An algorithm is wait free if every individual thread finishes in a bounded number of steps no matter what other threads do. (e.g. another thread could behave adversarily or simply block).

The problem is to have a wait free algorithm for atomic snapshots in a single writer multi-reader setup (the global state is a vector of values, each thread is the single writer for its values and other threads can read everyone elses values). A snapshot is an instance of the global state and it is atomic in that any snapshot returned refers to a set of values that actually existed simultaneously at some point in time (as opposed to composed from incomplete reads of multiple states). Snapshots so produced by an algorithm are linearizable - i.e. they form a total order on time rather than a partial order). Creating an algorithm that is not wait-free is trivial - you maintain reader-writer locks around the individual values of global state, however writers that block indefinitely would cause readers to starve.

### O(n^2) snapshots with unbounded Registers
Every thread should attach a unique seq. no. to its values. When readers see that the global state has not changed between two observations they can conclude that that represents an atomic snapshot. This is still not-wait free because busy writers can starve readers. The next thing is to force writers to perform a snapshot every time they must write a value and attach that snapshot to the value that they write.
The local state of a thread is now a `register` that contains its `value, seq. no. and snapshot`.
The snapshot operation can now succeed in two cases:
 - a double collect (two identical snapshot observations)
 - `s3` when a thread observes a sequence of 3 distinct values s1, s2, s3 from the same process.
This is wait free and a snapshot operation will terminate after 'n + 1' collects. This is because in order to prevent case (a) from being satisfied, a new value must be supplied at least every collect but each writer can provide only a single new value for otherwise case (2) is hit (s1 is the initial collect observation), there in the worst case there is the: initial collect + n - 1 collects + duplicate collect.
A collect operation involves examining the sequence nos. for each thread and takes 'n' steps so a snapshot takes O(n^2) in the worst case.

We also need to argue that this is atomic: they are is that new snapshots are only produced on double collects which are atomic. Otherwise we reuse old snapshots which by induction are atomic.
In practice writers will wait until they observe a double collect.

For wait free atomic snapshots, we want:
 - all snapshotters terminate in a finite number of steps - empircally true (and justified above)
 - if a snap s1 finishes before snap s2 begins then all seq nos. of s2's snap should be > s1
 - the snap should represent a value that actually occurred at some point during the interval between which the snap started and ended
 - updates should eventually make their way into a snap.

# Results

I ran some benchmarks with the following params:
Num procs x Number of snaps [8, 16, 32, 40, 48, 64, 72, 80] x [100, 200, 400, 800, 1600].
Results are under `analysis/`. It also contains the notebook with graphs illustrating the relationship between the various parameters.
From the graphs - the total execution time increases roughly linearly - as long as it's not competing with os for resources.
Another (expected) observation is that the number of collects increases with the number of processes simulatneously snapping stuff. Every 80 new processes collecting, 1 more collect needs to be performed on average per snapshot.

The collect time strangely doesn't increase linearly although it's basically a linear increase in the number of atomic loads and stores.

The number of Case A counts also for each initially increases and after about 60 simultaneous snappers processes it starts decreasing.


```
# Example of interleaving for a double collect fail
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662761 Thread-5248: START SNAP
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662763 Thread-5246: COLLECT END
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662766 Thread-5248: COLLECT BEGIN
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662769 Thread-5246: CASE A - 1
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662770 Thread-5248: COLLECT END
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662772 Thread-5246: END SNAP
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662772 Thread-5248: COLLECT BEGIN
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662776 Thread-5246: END UPDATE
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662778 Thread-5246: START SNAP
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662779 Thread-5246: COLLECT BEGIN
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662781 Thread-5246: COLLECT END
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662783 Thread-5246: COLLECT BEGIN
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662787 Thread-5246: COLLECT END
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662795 Thread-5248: COLLECT END
unbounded_regs-abb7053b043010ba1c9a631c69f622278d57e539-Mon_Oct_22_18-55-00_UTC_2018 1540234500662798 Thread-5248: DOUBLE COLLECT FAIL - 1
```

## Analysis Notes
Should perform these checks for both read heavy/write heavy workloads.
We should compare this against a locking implementation
The X-axis number of processes.
The Y-axis:
 - Snapshot completion time (nanoseconds)
 - Update time

Other interesting characteristics:
X-axis number of processes
Y-axis:
 - ratio of collects to snapshot attempts
 - time to collect
 - % of times when a snapshot ends with generating a new one
 - % of times when a snapshot ends with using an old one
 - Time between a write being made and it manifesting in a snapshot
(relationship between the last one and the probability of a new snapshot + snapshot time
collects/snapshot - %of times generating a new snap
snapshot completion time - number of collects per snap attempt
)



## Intel IA 32-64 notes on atomicity
Reference: https://software.intel.com/sites/default/files/managed/39/c5/325462-sdm-vol-1-2abcd-3abcd.pdf
Chapter 8:
```The processor uses three interdependent mechanisms for carrying out locked atomic operations:
• Guaranteed atomic operations
• Bus locking, using the LOCK# signal and the LOCK instruction prefix
8-2 Vol. 3A
MULTIPLE-PROCESSOR MANAGEMENT
• Cache coherency protocols that ensure that atomic operations can be carried out on cached data structures
(cache lock)```

```
The Intel486 processor (and newer processors since) guarantees that the following basic memory operations will
always be carried out atomically:
• Reading or writing a byte
• Reading or writing a word aligned on a 16-bit boundary
• Reading or writing a doubleword aligned on a 32-bit boundary
The Pentium processor (and newer processors since) guarantees that the following additional memory operations
will always be carried out atomically:
• Reading or writing a quadword aligned on a 64-bit boundary
• 16-bit accesses to uncached memory locations that fit within a 32-bit data bus
The P6 family processors (and newer processors since) guarantee that the following additional memory operation
will always be carried out atomically:
• Unaligned 16-, 32-, and 64-bit accesses to cached memory that fit within a cache line```

http://cbloomrants.blogspot.com/2014/11/11-11-14-x64-movdqa-atomic-test.html

It also provides 128 bit registers for dealing with packed integers in SSE2.
GCC exposes operations on these via intrinsics.

Writing to a 128 bit int with one bit set from different threads each writing the int with a different int set gives the following count. (the threads dont wait for the others to start)

I did some analysis on all possible interleavings of two thread writers and a checker and got the following results:
```
Starting state: (One, Zero)
fromList [((One,One),709632),((One,Zero),451584),((Zero,One),1032192),((Zero,Zero),709632)]

Starting state: (Zero, One)
fromList [((One,One),709632),((One,Zero),1032192),((Zero,One),451584),((Zero,Zero),709632)]

Starting state: (Zero, Zero)
fromList [((One,One),451584),((One,Zero),709632),((Zero,One),709632),((Zero,Zero),1032192)]

Starting state: (One, One)
fromList [((One,One),1032192),((One,Zero),709632),((Zero,One),709632),((Zero,Zero),451584)]
```
So given an initial state (x, y)
-> (x, y) = 35.5%
-> (x, not y) & (not x, y) = 24.4 (48.8%)
-> (not x, not y) = 15.55%

Looking at the actual results:

Test1: Lock around reads and writes
[jcoutin@loydjcoutin1 experiment]$ ./a.out
```
MAIN: Started my checker thread
Waking everyone up.
CHECKER: Started
(Iterations: 1000000, Num threads: 128) - Count: 0 - 0 (0.000000), 1 - 840562 (100.000238), 2 - 0 (0.000000)
```
This gives us what we expect - atomic reads and writes give us always just 1 bit.


All threads start synchronously using a monitor lock:
Test2: No lock,  [100000, 10000000, 100000000] iterations and [128, 256, 512, 1024] threads
(starting state: 1, iterations: 1000000, num threads: 128): run over 100 iterations, gives around 0.00001% chance of detecting a non-atomic write
(either 0 or 2 bits set).

There are about 60 instructions in the loop.

When locking the value for reading, the distribution of 0s - 1s looks very accurate (only 1 bit is set in a byte out of 4).
(Starting State: 1, Iterations: 1000000, Num threads: 128) - Count: 0 - 0 (0.000000), 1 - 46 (100.000000), 2 - 0 (0.000000)
(
Num 0s, Num 1s, pc 0s, pc 1s)
[Byte 0] (35, 11, 76.086957, 23.913043)
[Byte 1] (36, 10, 78.260870, 21.739130)
[Byte 2] (34, 12, 73.913043, 26.086957)
[Byte 3] (33, 13, 71.739130, 28.260870)

Randomly set two bits not in the same byte locking around the value:
```
(Starting State: 1, Iterations: 1000000, Num threads: 128) - Count: 0 - 0 (0.000000), 1 - 0 (0.000000), 2 - 813 (100.000000), 3 - 0 (0.000000), 4 - 0 (0.000000)
(
Num 0s, Num 1s, pc 0s, pc 1s)
[Byte 0] (391, 422, 48.093481, 51.906519)
[Byte 1] (396, 417, 48.708487, 51.291513)
[Byte 2] (423, 390, 52.029520, 47.970480)
[Byte 3] (416, 397, 51.168512, 48.831488)
```

Unlocked racing access when setting two bits on disjoint bytes:
```
All done.
(Starting State: 1, Iterations: 1000000, Num threads: 128) - Count: 0 - 3 (0.000218), 1 - 31 (0.002252), 2 - 1376451 (99.993825), 3 - 44 (0.003196), 4 - 7 (0.000509)
Num 0s, Num 1s, pc 0s, pc 1s)
[Byte 0] (691361, 685175, 50.224694, 49.775306)
[Byte 1] (648908, 727628, 47.140649, 52.859351)
[Byte 2] (733807, 642729, 53.308232, 46.691768)
[Byte 3] (678975, 697561, 49.324900, 50.675100)
```

With an atomic compare and exchange it looks like its actually atomic.
```
All done.
(Starting State: 1, Iterations: 5000000, Num threads: 128) - Count: 0 - 0 (0.000000), 1 - 0 (0.000000), 2 - 8217932 (100.000000), 3 - 0 (0.000000), 4 - 0 (0.000000)
(
Num 0s, Num 1s, pc 0s, pc 1s)
[Byte 0] (4143038, 4074894, 50.414606, 49.585394)
[Byte 1] (3992507, 4225425, 48.582867, 51.417133)
[Byte 2] (4229243, 3988689, 51.463592, 48.536408)
[Byte 3] (4071076, 4146856, 49.538935, 50.461065)
```
Things to note - you need to include the atomic library, there is no 16byte load and store just cmpxchg (which is strong enough for both). Setting half the bytes is best because of more of a chance of observing race conditions and getting a normal distribution.




#### Questions
Does this work when all of them are talking about the same object (does it give a consistent picture) - no they're meant to be distinct although commutative associative operations work fine (sum, max, min etc.)




# Lockless programming
https://docs.microsoft.com/en-us/windows/desktop/DxTechArts/lockless-programming

Another good example: https://preshing.com/20120612/an-introduction-to-lock-free-programming/

Issues to deal with, non-atomic operations and re-ordering of instructions.
'inc' operations are non-atomic for a multi-core system but are atomic on a single core system. (read, write incremented value). Reads are not usually re-orderderd relative to other reads and the same for writes. Reads may be re-ordered relative to other writes.
The main constructs used to prevent reordering of reads and writes are called read-acquire and write-release barriers. A read-acquire is a read of a flag or other variable to gain ownership of a resource, coupled with a barrier against reordering. Similarly, a write-release is a write of a flag or other variable to give away ownership of a resource, coupled with a barrier against reordering.
The formal definitions, courtesy of Herb Sutter, are:

 - A read-acquire executes before all reads and writes by the same thread that follow it in program order.
 - A write-release executes after all reads and writes by the same thread that precede it in program order.

If a barrier is not used an example where data-sharing may fail is the following:
A read to some part of the resource for a read acquire moves ahead of the release from a write release.

Note, all this synchronization only makes sense in the context of cacheable memory. Non cacheable memory is not affected by instruction reordering. The volatile keyword is used to guarentee that the memory location is read/written to directly from memory is not cached. This doesn't mitigate instruction reordering though.


Example of a possible case for live-lock:
```
while (X == 0)
{
    X = 1 - X;
}
```
If t1 and t2 constently perform the update right after each other they will never break out of the loop.

Different compiler/processor optimizations: caching, instruction pre-fecthing, branch prediction, instruction re-ordering.

# Project setup:
`build.sh` to rebuild. `run.sh` to run the project. It generates a log file to /logs/{run-name}. `/logs` is usually mounted to `~/josh/projects/logs/{project-name}` (shoul use a `volume` for this instead). Logstash runs on that directory parsing those files uploading to my elasticsearch cluster. The run-name consists of the {app name}-{commit id}-{time stamp}. I've put some processed logs inside `~/josh/projects/processed_logs`. `/code/libs` contains all my libs.


