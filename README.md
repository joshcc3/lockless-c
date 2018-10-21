# Atomic Snapshots

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

Starting state: (One, Zero)
fromList [((One,One),709632),((One,Zero),451584),((Zero,One),1032192),((Zero,Zero),709632)]

Starting state: (Zero, One)
fromList [((One,One),709632),((One,Zero),1032192),((Zero,One),451584),((Zero,Zero),709632)]

Starting state: (Zero, Zero)
fromList [((One,One),451584),((One,Zero),709632),((Zero,One),709632),((Zero,Zero),1032192)]

Starting state: (One, One)
fromList [((One,One),1032192),((One,Zero),709632),((Zero,One),709632),((Zero,Zero),451584)]

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


## Wait free
An algorithm is wait free if every individual thread finishes in a bounded number of steps no matter what other threads do. (e.g. another thread could behave adversarily or simply block).

References are this papers  http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.406.4248&rep=rep1&type=pdf and http://www.cs.yale.edu/homes/aspnes/pinewiki/AtomicSnapshots.html#The_Gang_of_Six_algorithm
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

## Performance analysis:
```
# Some python code to parse the log lines
def mkline(line):
    headers = ["app", "commitid", "datefmt", "unix_tstamp", 'tid', 'message']    
    return dict(zip(headers, regex.match(line).groups()))

def process_log_file(fname):
    content = open('../../../logs/{}'.format(fname)).read()
    regex = re.compile('([a-zA-Z0-9_]*)-([0-9a-zA-Z]*)-(.*) (\d*) Thread-(\d*): (.*)')     
    log_lines = pd.DataFrame(map_(mkline)(content.splitlines()))    
    log_lines.to_csv('/Users/jrc12/josh/projects/processed_logs/{}'.format(fname))
    return log_lines
```
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

I ran some benchmarks with the following params:
Num procs x Number of snaps [10, 100, 1000] x [10000, 100000, 1000000].
I ran each of the tests 20 times - results are under `analysis/`. It also contains the notebook with graphs illustrating the relationship between the various parameters.




#### Questions
Does this work when all of them are talking about the same object (does it give a consistent picture) - no they're meant to be distinct although commutative associative operations work fine (sum, max, min etc.)


# Threading interface
## PThread Interface
The type pthread_t is opaque - its an 8 byte identifier that is used internally. pthread_mutex_t and pthread_cond_t on ther other hand have the following properties:
```
(gdb) p lock
$5 = {__data = {__lock = 0, __count = 0, __owner = 0, __nusers = 5, __kind = 0, __spins = 0, __elision = 0, __list = {__prev = 0x0,
      __next = 0x0}}, __size = '\000' <repeats 12 times>, "\005", '\000' <repeats 26 times>, __align = 0}


(gdb) p cond
$6 = {__data = {__lock = 0, __futex = 5, __total_seq = 5, __wakeup_seq = 0, __woken_seq = 0, __mutex = 0x7fffffffe180,
    __nwaiters = 10, __broadcast_seq = 0},
  __size = "\000\000\000\000\005\000\000\000\005", '\000' <repeats 23 times>, "\200\341\377\377\377\177\000\000\n\000\000\000\000\000\000", __align = 21474836480}
```
On linux in glibc, both conditions and locks are implemented with futexes.
Looking at the kernel sources for normal mutexes: 
https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=nptl/pthread_mutex_lock.c;hb=HEAD
simple:
      /* Normal mutex.  */
      LLL_MUTEX_LOCK (mutex);
      assert (mutex->__data.__owner == 0);

# define LLL_MUTEX_LOCK(mutex) \
  lll_lock ((mutex)->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex))      

#define lll_lock(futex, private) __lll_lock (&(futex), private)

#define __lll_lock(futex, private)					      \
  ((void) ({								      \
    int *__futex = (futex);						      \
    if (__builtin_expect (atomic_compare_and_exchange_val_acq (__futex,       \
								1, 0), 0))    \
      {									      \
	if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)	      \
	  __lll_lock_wait_private (__futex);				      \
	else								      \
	  __lll_lock_wait (__futex, private);				      \
      }									      \
  }))


# define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_compare_and_exchange_val,acq,		      \
		       mem, newval, oldval)


__lll_lock_wait (int *futex, int private)
{
  if (*futex == 2)
    lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */
  while (atomic_exchange_acq (futex, 2) != 0)
    lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */
}

https://chromium.googlesource.com/chromiumos/third_party/glibc-ports/+/factory-2368.B/sysdeps/unix/sysv/linux/arm/nptl/lowlevellock.h for the defintion of lll_lock(futex, private)


It has 3 states, 0 - unlocked, 1 - contended, 2 - locked

Common threading interface exposed in unix and is documented in the man pages.
### pthread_create
```
       int pthread_create(pthread_t *restrict thread,
              const pthread_attr_t *restrict attr,
              void *(*start_routine)(void*), void *restrict arg);
```
This creates and starts the thread calling start_routine with arg. 'thread' is populated with the ID.
On successful completion 0 is returned. The call can fail when there aren't enough resources/you don't have permission to perform the requested attributes or they are invalid.
Returning from 'start_routine' implicitly calls pthread_exit with the return value. (although the main function implicitly calls exit())

### pthread_cancel
       int pthread_cancel(pthread_t thread);
Runs the cancellation handlers, thread data destructors and then the thread is terminated. (not sure where these handlers are defined).

p_thread_exit should be called on main because it will block on the child threads (whereas otherwise with a call to exit all the children would be killed).
p_thread_join - waits for the target to exit, and if the value_ptr is non-null it gets the result of the called thread. This can return EDEADLK.

### mutes_init and destry
cannot reinitialize an undestroyed mutex, cannot destroy a locked mutex. (Get the EBUSY error).

### pthread_mutex_lock, trylock, unlock
There are different types of mutexes - default, error check, recursive, normal
Recursive ones are re-entrant. error check returns error values on error conditions - unlocking something unlocked, locking somethhing you own
Signals are attended to by processes waiting on mutex and then they go back to waiting.
Trylock returns an EBUSY and does not block if the mutex is locked.
The scheduling policy determines which thread gets access to the mutex when unlock is called and threads are waiting.


## Experiments
```

void* thread_action(void * null_arg)
{
  printf("Start!\n");
  sleep(2);
  printf("End!\n");
}


int main()
{
  pthread_t tid1;
  int res = pthread_create(&tid1, NULL, thread_action, NULL);
  printf("Main thread exits\n");
  pthread_exit(NULL);
}
////
[jcoutin@loydjcoutin1 concurrent]$ gcc test.c -lpthread
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
Start!
End!
```

Main waits on its child before exiting.

```
// when you remove the pthread_exit(NULL) it prints out Start!, Start! twice without fail

void* thread_action(void * null_arg)
{
  printf("Start!\n");
  sleep(2);
  printf("End!\n");
}


int main()
{
  pthread_t tid1;
  int res = pthread_create(&tid1, NULL, thread_action, NULL);
  printf("Main thread exits\n");
  //pthread_exit(NULL);
}

//////
[jcoutin@loydjcoutin1 concurrent]$ gcc test.c -lpthread
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
Start!
Start!
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
[jcoutin@loydjcoutin1 concurrent]$ ./a.out
Main thread exits
Start!
Start!

```

```

/*
what happens to a thread when the parent dies:
  parent is the main function - the child is killed - there can be unintended effects (for example I think the pc of the child thread got overwritten and it reverted to a previous instruction?)
  parent is not the main function (and we pthread_exit in the main function) - the child is not killed and is preserved.

  Does calling pthread_exit in the parent thread wait for the child thread? (not main)  - yes it does
  Does calling pthread_exit in the parent thread wait for the grand child thread? (not main) - yes all children in the thread group are waited for if you call pthread_exit somewhere in the hierarchy

  Inference, it's only the exit syscall that actually clears stuff up.

if a thread joins on a deceased thread does is still capture its value?
 */




void* grand_child_thread(void *null_arg)
{
  printf("Grand Child thread about to sleep.\n");
  sleep(2);
  printf("Grand Child about to exit.\n");
}
void* child_thread(void *null_arg)
{
  pthread_t grand_t;
  pthread_create(&grand_t, NULL, grand_child_thread, NULL);
  printf("Child Exiting!\n");
}

void* parent_thread(void * null_arg)
{
  pthread_t child_t;
  pthread_create(&child_t, NULL, child_thread, NULL);
  printf("Parent thread exiting!\n");
}


int main()
{
  pthread_t tid1;
  int res = pthread_create(&tid1, NULL, parent_thread, NULL);
  printf("Main thread pexits\n");
  pthread_exit(NULL);
}
```

```
// any thread can join join on any other thread.
void* branch1(void *return_val)                                                                                                        |[jcoutin@loydjcoutin1 concurrent]$ gcc -ggdb -pthread test.c
{                                                                                                                                      |[jcoutin@loydjcoutin1 concurrent]$
  printf("Branch1 sleeping for 5 seconds\n");                                                                                          |[jcoutin@loydjcoutin1 concurrent]$
  sleep(5);                                                                                                                            |[jcoutin@loydjcoutin1 concurrent]$
  *(int*)return_val = 10;                                                                                                              |[jcoutin@loydjcoutin1 concurrent]$ ./a.out
  printf("Branch1 exiting\n");                                                                                                         |Branch1 sleeping for 5 seconds
  return return_val;                                                                                                                   |Branch2 joining branch1
}                                                                                                                                      |a.out: test.c:84: main: Assertion `res == 10' failed.
                                                                                                                                       |Aborted
void* branch2(void *thread_to_wait)                                                                                                    |[jcoutin@loydjcoutin1 concurrent]$
{                                                                                                                                      |[jcoutin@loydjcoutin1 concurrent]$
  int *result;                                                                                                                         |[jcoutin@loydjcoutin1 concurrent]$ gcc -ggdb -pthread test.c
  pthread_t *branch1 = (pthread_t*)thread_to_wait;                                                                                     |[jcoutin@loydjcoutin1 concurrent]$ ./a.out
  printf("Branch2 joining branch1\n");                                                                                                 |Branch1 sleeping for 5 seconds
  pthread_join(*branch1, (void**)&result);                                                                                             |Branch2 joining branch1
  printf("Branch2 reports that the return value of branch1 is %d\n", *result);                                                         |Branch1 exiting
  assert(*result == 10);                                                                                                               |Branch2 reports that the return value of branch1 is 10
  return NULL;                                                                                                                         |[jcoutin@loydjcoutin1 concurrent]$
}                                                                                                                                      |
                                                                                                                                       |
int main()                                                                                                                             |
{                                                                                                                                      |
  pthread_t b1;                                                                                                                        |
  pthread_t b2;                                                                                                                        |
                                                                                                                                       |
  int res;                                                                                                                             |
                                                                                                                                       |
  pthread_create(&b1, NULL, branch1, (void*)&res);                                                                                     |
  pthread_create(&b2, NULL, branch2, &b1);                                                                                             |
                                                                                                                                       |
  pthread_exit(NULL);                                                                                                                  |
                                                                                                                                       |
}
```

### pthread_cond_wait

cond_wait takes a mutex and a cond_wait object. A pre-condition is that the calling thread must hold the lock on the mutex. The operation of cond_wait is to atomically release the mutex and sleep on the condition variable. The thread wakes up when a cond_signal/cond_broadcast has been sent. It then attempts to reacquire the mutex.
When the thread returns from pthread_cond_wait:
Throughout the lifetime of the pthread_cond_wait function, it will have held the lock, then blocked on cond, then woken up on a signal and then acquired the mutex.
The reason for the atomic release of the cond var and acquisition of the mutex is that if something wants to broadcast a signal to threads waiting on a signal you can guarentee that the broadcast signal will reach everyone who has started waiting on the signal.


## Question
 - Are the signals persistent? If a signal has been sent will something that subsequently wait on it be immediately woken up? - No, The pthread_cond_signal() and pthread_cond_broadcast() functions have no effect if there are no threads currently blocked on cond.
 - Can multiple threads block on the mutex? of course, otherwise there would be no reason for a broadcast.
 - Are the threads woken up in order?

conds are associated with a boolean predicate. Spurious wake ups may occur and the condition variable should always be checked before proceeding.
The reason that conds are always associated with a boolean predicate and not just for sending signals is that signals are lost. 

An example use of monitors might be to synchronize the starting of various threads so that they all begin together
```

const int NUM_THREADS = 5;
typedef struct monitor {
  pthread_cond_t *condition_var;
  pthread_mutex_t *lock;
} monitor;

typedef struct waiting_thread {
  monitor ready_monitor;
  int *thread_ready_count;
} waiting_thread;

void* waiter_function(void *args_)
{
  pthread_t thread_id = pthread_self();
  waiting_thread *args = (struct waiting_thread*)args_;
  int sleep_time = (int)(rand()/(float)RAND_MAX * 10);
  printf("TID %x: Sleeping for %d\n", thread_id, sleep_time);

  sleep(sleep_time);

  // guarentee
  // 1. The broadcast signal is only sent when all threads have woken up

  printf("TID %x: About to enter the mutex to increment thread ready count\n", thread_id);
  pthread_mutex_lock(args->ready_monitor.lock);
  *args->thread_ready_count += 1;
  if(*args->thread_ready_count == NUM_THREADS)
  {
    printf("TID %x: All %d have started, sending a broadcast signal\n", thread_id, *args->thread_ready_count);
    pthread_cond_broadcast(args->ready_monitor.condition_var);
  }
  else
  {
    printf("TID %x: Waiting for all threads to be ready, currently only %d have started\n", thread_id, *args->thread_ready_count);
    pthread_cond_wait(args->ready_monitor.condition_var, args->ready_monitor.lock);
    printf("TID %x: just received a signal, woken up and am plowing on\n", thread_id);
  };
  pthread_mutex_unlock(args->ready_monitor.lock);

}


void init_monitor(monitor *monitor)
{
  monitor->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  monitor->condition_var = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

  pthread_cond_init(monitor->condition_var, NULL);
  pthread_mutex_init(monitor->lock, NULL);

}

int main()
{



  // test order of wakeups on condition signal
  int thread_ready_count = 0;

  monitor ready_monitor;
  init_monitor(&ready_monitor);

  struct waiting_thread ts;
  ts.ready_monitor = ready_monitor;
  ts.thread_ready_count = &thread_ready_count;

  pthread_t tids[NUM_THREADS];


  printf("MAIN: Spawning the threads\n");
  for(int i = 0; i < NUM_THREADS; i++)
  {
    pthread_t *t = tids + i;
    pthread_create(t, NULL, waiter_function, (void*)&ts);
  }
  printf("MAIN: Completed spawning %d threads\n", NUM_THREADS);
  pthread_mutex_lock(ts.ready_monitor.lock);
  while(*ts.thread_ready_count < NUM_THREADS)
  {
    printf("MAIN: Waiting for the all clear signal\n");
    pthread_cond_wait(ts.ready_monitor.condition_var, ts.ready_monitor.lock);
  }
  pthread_mutex_unlock(ts.ready_monitor.lock);

  printf("MAIN: Received the all clear, plowing ahead!\n");

  pthread_exit(NULL);
}
```


## The `time` function in linux
All experiments were done on:
```
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 79
model name      : Intel(R) Xeon(R) CPU E5-2697 v4 @ 2.30GHz
stepping        : 1
microcode       : 0xb000025
cpu MHz         : 2294.686
cache size      : 46080 KB
physical id     : 0
siblings        : 1
core id         : 0
cpu cores       : 1
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes

On linux version: 3.10.0-693.11.6.el7.x86_64 #1 SMP 
```

Time outputs something like this:
```
real    0m20.919s
user    0m11.118s
sys     0m15.768s
```

The `times` system call on linux reports the user and system time taken for the process and its children in clock ticks. clock ticks -> seconds is OS dependent and can be retrieved by sysconf(_SC_CLK_TCK). Although I found a definite disparity in their reporting:
```
[jcoutin@loydjcoutin1 experiment]$ ./a.out
User time: 0.000000 us, System Time: 0.000000 us
[jcoutin@loydjcoutin1 experiment]$ time ./a.out
User time: 0.000000 us, System Time: 0.000000 us

real    0m1.003s
user    0m0.002s
sys     0m0.001s
```
Thats the the wall clock time, time spent executing user instructions and time spent in the kernel for the process.
What is the time distribution for the following scenarios? (2 processors) [The code to test these sections is present inside `c_src/experiment/test_times.c`).

### When a process sleeps for 1 second
It registers as 0 from the perspective of times (I guess it literally hasn't taken enough of cpu time to really register in terms of clock ticks!)
### Looping through an array
Something weird:
```
  long large_sum = 0;
  for(int i = 0; i < 10e6; i++)
    {
      large_sum += i;
    }

  times(&tms);
  pprint_tms(&tms);
  /*
  $ time ./a.out
RAW: Clock Ticks - User time: 3, System Time: 0
User time: 30.000000 ms, System Time: 0.000000 ms

real    0m0.035s
user    0m0.034s
sys     0m0.002s

But changing the 10e6 to 1000000 gives me 0 on the user time and system time:
Relevant assembly before change: (its cut 10e6 is actually 10^7) 
.L30:
        mov     eax, DWORD PTR [rbp-12]
        cdqe
        add     QWORD PTR [rbp-8], rax
        add     DWORD PTR [rbp-12], 1
.L29:
        cvtsi2sd        xmm0, DWORD PTR [rbp-12]
        movsd   xmm1, QWORD PTR .LC12[rip]
        ucomisd xmm1, xmm0
        ja      .L30


Relevant assembly after change:
.L30:
        mov     eax, DWORD PTR [rbp-12]
        cdqe
        add     QWORD PTR [rbp-8], rax
        add     DWORD PTR [rbp-12], 1
.L29:
        cmp     DWORD PTR [rbp-12], 999999
        jle     .L30

*/
```

Also A tick is an arbitrary unit for measuring internal system time. There is usually an OS-internal counter for ticks; the current time and date used by various functions of the OS are derived from that counter.
The tick on the machine that I'm using (Linux 3.10 x64 is 1/100th of a second - so it only has accuracy upto a 10 milliseconds).

### Adding a number:
```
  long large_sum = 0;
  for(long i = 0; i < 1000000000; i++)
    {
      large_sum += i;
    }

///
.L30:
        mov     rax, QWORD PTR [rbp-16]
        add     QWORD PTR [rbp-8], rax
        add     QWORD PTR [rbp-16], 1
.L29:
        cmp     QWORD PTR [rbp-16], 9999999
        jle     .L30
```
User time: 2610.000000 ms, System Time: 0.000000 ms
User time: 2630.000000 ms, System Time: 0.000000 ms
User time: 2610.000000 ms, System Time: 0.000000 ms
User time: 2600.000000 ms, System Time: 0.000000 ms
User time: 2610.000000 ms, System Time: 0.000000 ms
User time: 2590.000000 ms, System Time: 0.000000 ms
User time: 2620.000000 ms, System Time: 0.000000 ms
User time: 2610.000000 ms, System Time: 0.000000 ms
User time: 2600.000000 ms, System Time: 0.000000 ms
User time: 2590.000000 ms, System Time: 0.000000 ms
User time: 2630.000000 ms, System Time: 0.000000 ms
Mean: 2609, std dev: 13

So (load, add, add, cmp, jmp) cycle takes 2609/10^9 seconds which is 3microseconds.

Adding in an additional increment to a variable I get:
User time: 2720.000000 ms, System Time: 0.000000 ms
User time: 2810.000000 ms, System Time: 0.000000 ms
User time: 3030.000000 ms, System Time: 0.000000 ms
User time: 2730.000000 ms, System Time: 0.000000 ms
User time: 2810.000000 ms, System Time: 0.000000 ms
User time: 2800.000000 ms, System Time: 0.000000 ms
User time: 2690.000000 ms, System Time: 0.000000 ms
User time: 2810.000000 ms, System Time: 0.000000 ms
User time: 2720.000000 ms, System Time: 0.000000 ms
User time: 2720.000000 ms, System Time: 0.000000 ms
User time: 2790.000000 ms, System Time: 0.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2810.000000 ms, System Time: 0.000000 ms
User time: 2750.000000 ms, System Time: 0.000000 ms
User time: 2810.000000 ms, System Time: 10.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2740.000000 ms, System Time: 0.000000 ms
User time: 2750.000000 ms, System Time: 0.000000 ms
User time: 2740.000000 ms, System Time: 0.000000 ms
User time: 2800.000000 ms, System Time: 0.000000 ms
Mean: 2779, Std Dev. 70

170ms difference which means that an invidual add instruction takes approximately 17 nanoseconds.
(Using (-) instead, there's no difference).
User time: 2690.000000 ms, System Time: 0.000000 ms
User time: 2720.000000 ms, System Time: 0.000000 ms
User time: 2770.000000 ms, System Time: 0.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2690.000000 ms, System Time: 0.000000 ms
User time: 2820.000000 ms, System Time: 0.000000 ms
User time: 2700.000000 ms, System Time: 0.000000 ms
User time: 2830.000000 ms, System Time: 0.000000 ms
User time: 2820.000000 ms, System Time: 0.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2700.000000 ms, System Time: 0.000000 ms
User time: 2830.000000 ms, System Time: 0.000000 ms
User time: 2840.000000 ms, System Time: 0.000000 ms
User time: 2790.000000 ms, System Time: 0.000000 ms
User time: 2750.000000 ms, System Time: 0.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2860.000000 ms, System Time: 0.000000 ms
User time: 2750.000000 ms, System Time: 0.000000 ms
User time: 2780.000000 ms, System Time: 0.000000 ms
User time: 2820.000000 ms, System Time: 0.000000 ms
Mean 2775, Std Dev: 53

Creating a thread and noop does nothing (just pthread_exits)
```
  for(long i = 0; i < 100000; i++)
    {
      large_sum += i;
      pthread_create(&t, NULL, noop, &val);
    }
```
Without the pthread_creation:
real    0m0.003s
user    0m0.001s
sys     0m0.002s
With the pthread_creation and a child that does nothing:
real    0m3.554s
user    0m0.467s
sys     0m3.449s
With the pthread_creation and a child that sums numbers from 1 - 10^7:
real    7m20.416s
user    14m17.900s
sys     0m3.828s
With the pthread_creation and a child that sums numbers from 1 - 10^6:
real    0m47.152s
user    1m26.517s
sys     0m3.791s
Sums numbers from 1 - 10^5
real    0m8.715s
user    0m8.925s
sys     0m3.509s
Sums from 1 - 10^4
real    0m4.063s
user    0m1.336s
sys     0m3.570s


A thread takes 3ms to sum 10^6 numbers.
So, thread creation takes 4 microseconds of user time and 30 microseconds of system time based on the above.
Looking at the last table it's nice to see that system time does not change - since the threads dont actually do any system related work.
It's also nice to see that the user time grows by roughly a factor of 10 every time we multiply the number of ops a user thread is doing.
With the 10^4 loops:
10^6 ops - 3ms (I think the difference from before lies in the fact that the value must be in the cache - wasn't able to verify this by changing the update variable to volatile).
1 op - 3ns
10^4 ops = 30 us (micro) per thread, 10^5 threads, should be 3s total user time, just doing actual ops.
Then there's 450ms spent in thread creation which gives 3450ms of total user time. With two processors working perfectly that gives 1.725ms of total user time. However, the reporting suggests that it actually takes < 1.4ms. Which means that processing a loop actually takes around 9.5 microseconds.




Throughput:


How much time does a thread context switch take?
Number of threads versus throughput.
How does blocking IO work with threads?
 - Does a blocked thread get swapped in for a live one?



### What about simple things like printing.
    - How does the time change with the size of the string.
### Threads
 - Creating a thread
 - Joining a thread
### Mutexes
 - Creating a mutex
 - Taking a lock on a mutex with no contention
 - Taking a lock on a mutex with contention

How do the following distributions change when we move to a single processor? (you can use `taskset` to manipulate the 'cpu affinity' of existing and new processes)
### When a process sleeps for 1 second
### What about simple things like printing.
    - How does the time change with the size of the string.
### Threads
 - Creating a thread
 - Joining a thread
### Mutexes
 - Creating a mutex
 - Taking a lock on a mutex with no contention
 - Taking a lock on a mutex with contention



## Atomicity of updates to read:

All of the tests were tried 10000 times.
So splitting the read and write into two instructions gives a 93.42 chance of success with 10 threads and not splitting it has the same effect. 
```
/*
Splitting up the read and write
Total: 10000, Failed: 658, Correct: 9342, Percent Success: 93.420000

real    0m5.818s
user    0m0.883s
sys     0m5.416s
*/
void* incers(void *null)
{
  for(int i = 0; i < INC_COUNT; i++)
  {
    int read = protected;
    protected = read + 1;
  }

}


// Changing the update loop to
/*
Total: 10000, Failed: 326, Correct: 9674, Percent Success: 96.740000

real    0m5.754s
user    0m0.759s
sys     0m5.402s
*/
void* incers(void *null)
{
  for(int i = 0; i < INC_COUNT; i++)
  {
    //lock(&global_lock);
    //usleep(rand()*100);
    protected += 1;
    //unlock(&global_lock);
  }

}

// which compiles to: (changing to ++protected, makes no difference)
.L24:
        mov     eax, DWORD PTR protected[rip]
        add     eax, 1
        mov     DWORD PTR protected[rip], eax
        add     DWORD PTR [rbp-4], 1
.L23:
        mov     eax, 1000
        cmp     DWORD PTR [rbp-4], eax
        jl      .L24
(the 



// With a proper lock
void* incers(void *null)
{
  for(int i = 0; i < INC_COUNT; i++)
  {
    //lock(&global_lock);
    pthread_mutex_lock(&global);
    int read = protected;
    //usleep(rand()*100);
    protected = read + 1;
    pthread_mutex_unlock(&global);
    //unlock(&global_lock);
  }

}


Total: 10000, Failed: 0, Correct: 10000, Percent Success: 100.000000

real    0m7.599s
user    0m3.279s
sys     0m6.202s


```

Full code here:
```


int NUM_THREADS = 10;
int INC_COUNT = 1000;

void* incers(void *null)
{
  for(int i = 0; i < INC_COUNT; i++)
  {
    int read = protected;
    protected = read + 1;
  }

}


typedef struct stats {
  int correct;
  int total;
} stats;

void print_stats(stats st)
{
  printf("Total: %d, Failed: %d, Correct: %d, Percent Success: %f\n", st.total, st.total - st.correct, st.correct, 100.0*st.correct/(float)st.total);

}

int main()
{
  stats st = { .correct = 0, .total = 0 };


  for(int x = 0; x < 10000; x++){
    global_lock.state = 0;

    pthread_t ts[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++) pthread_create(ts + i, NULL, incers, NULL);

    for(int i = 0; i < NUM_THREADS; i++) pthread_join(ts[i], NULL);

    printf("Checking");
    if(protected != NUM_THREADS * INC_COUNT) {
      printf("Iteration: %d, Check Failed!: protected (%d) != NUM_THREADS*INC_COUNT (%d)\n", x, protected, NUM_THREADS * INC_COUNT);
    }
    else st.correct++;
    st.total++;
  }

  print_stats(st);

  pthread_exit(NULL);

}

```

When changing to compile with -O3, the loop is optimized and reduced to an add instruction (I'm not sure what the rip is doing in address but protected is a label). (Interestingly it doesn't use an `inc` instruction even if I remove the loop around it). It also does away with the saving of the base pointer and saving of the args on the stack.
```
add protected[rip], 1000`
incers:
.LFB13:
        .loc 1 82 0
        .cfi_startproc
.LVL17:
.LBB12:
        .loc 1 87 0
        add     DWORD PTR protected[rip], 1
.LVL18:
.LBE12:
        .loc 1 92 0
        ret
        .cfi_endproc
```

Running for 10000 iterations, with 200 threads:
Total: 10000, Failed: 446, Correct: 9554, Percent Success: 95.540000

So even an add instruction is not atomic across multiple processors.

When actually running this with `taskset --cpu-list 1 ./a.out` I get no test failures. So `add` is atomic when you have a single processor.
```
$ taskset --cpu-list 1 ./a.out
Total: 1000, Failed: 0, Correct: 1000, Percent Success: 100.000000

real    0m16.085s
user    0m1.152s
sys     0m11.765s


$ time taskset --cpu-list 0,1 ./a.out
Total: 1000, Failed: 49, Correct: 951, Percent Success: 95.100000

real    0m13.067s
user    0m1.270s
sys     0m12.994s

```
Looks like a massive amount of time is also spent creating the threads. 1000*200 threads were created here (around 60 microseconds to create a new thread?).

## Implementing my own locks:
For a basic CAS:
```
bool compare_and_swap(lock_t *lock, int a, int b)                               
{                                                                               
  pthread_mutex_lock(&cas_lock);                                                
  if(lock->state != a) {                                                        
    pthread_mutex_unlock(&cas_lock);                                            
    return 0;                                                                   
  }                                                                             
  lock->state = b;                                                              
                                                                                
  pthread_mutex_unlock(&cas_lock);                                              
                                                                                
  return 1;                                                                     
                                                                                
}
                                                                                
//The lock function:
  while(1)                                                                      
  {                                                                             
    if(lock->state == 0)                                                        
    {                                                                           
      compare_and_swap(lock, 0, 1);                                             
    }                                                                           
    else if(lock->state == 1)                                                   
    {                                                                           
      bool result = compare_and_swap(lock, 1, 2);                               
      if(result) break;                                                         
    }                                                                           
    else sleep(SLEEP_INTERVAL);                                                 
  }   

// Unlock just sets the lock->state = 0

$ time ./a.out
Starting
Completed test.Total: 1000, Failed: 0, Correct: 1000, Percent Success: 100.000000

real    0m20.919s
user    0m11.118s
sys     0m15.768s


```

### How strong a CAS do you really need to implement a lock?



### Evaluating concurrent algorithms:

Throughput - Number of non-locking operations
Lock contention:
 - Number of times a thread sleeps





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

# Dev in C

First write a test to see what you expect the interface to look like.
Then write out the functions that describe the behaviour. Then write out the tests.
Then think out the pre and post conditions.

# Makefiles
Variables are specified as <key> := <value>. The convention is to use capitals for keys.
You refer to variables with $(<key>)

The usualy layout is to have an all target and a clean target at least. clean is marked as .PHONY to note that there is nothing generated (line above the target `.PHONY clean:`). The generated files go into a target directory whose internal dir structure matches the source dir structure.
The actions of the rule must start with a `tab`.
A rule consists of a: (the deps are other targets)
<target>: <deps>
<tab>shell commands
The dependencies must either be an existing target or an existing file.
An example rule to compile a .o files from the corresponding .c file. The % denote variables that are captured in rule local variables.
The targets are captured in $@ and the dependency in $^. There are more variables and funnier ways of using them.
Apparently the `%` only works to match against an individual file line and cannot match against an arbitrary path so you can only glob files in a dir?
The `-c` flag just compiles into an '.o' or '.s' but does not link otherwise it tries to create an executable. The `-I` flag only specifies where to look for header files. You need to give the absolute path to all the files needed for compilation. You can get environment variables with ${env_name}.
```
test/stg/target/stg/plus_int/%.o: src/stg/plus_int/%.c
	mkdir -p test/stg/target/stg/plus_int
	$(GCC_CMD) $^ -c -o $@
```
For the main executable you would have the exec name followed by the object file deps.
```
test/stg/target/main3: test/stg/target/stg/plus_int/code.o test/stg/target/stg/plus_int/stack.o test/stg/target/stg/plus_int/static.o test/stg/target/stg/list/list.o \
        test/stg/target/stg/heap.o \
		test/data/target/data/string_.o test/containers/target/containers/mmanager.o test/containers/target/containers/arraylist.o\
		     test/stg/target/stg/math.o test/stg/target/stg/bindings.o test/target/typeclasses.o test/stg/target/stg/util.o test/containers/target/containers/llist.o\
		   test/containers/target/containers/hash_set.o test/containers/target/containers/resizable_array.o test/containers/target/containers/hash_map.o \
           test/stg/target/stg/main3.o
	$(GCC_CMD) -I . $^ -o $@

```

All top level lines need to end with a `:`. (including phone decls.)

You can include the result of shell operations with $(shell [cmd]). You can perform variable substitution for everything inside a variable with
```
foo := a.o b.o
bar := $(foo:%.c=%.o)

For some reasons when compiling with gcc, sometimes the libraries need to be mentioned at the end of the compile command. Also, `-lpthread -lm -latomic -Llibs -ljc` is wrong.
The `-ljc` which depends on `-latomic` needs to be mentioned before it for some reason otherwise it can't find it.

# bar contains a.c b.c
```

# Oh, No!
## __builtin_popcount only works for up to 32 bits and doesn't actually work on 64/128 bits
## I crashed ghc :/
*Main> aggregate $ map (processState oneZero) possibleOutcomes
fromList [((One,One),709632),((One,Zero),1032192),((Zero,One),451584),((Zero,Zero),709632)]
*Main>
ghc.exe: panic! (the 'impossible' happened)
  (GHC version 8.0.2 for x86_64-unknown-mingw32):
        thread blocked indefinitely in an MVar operation
