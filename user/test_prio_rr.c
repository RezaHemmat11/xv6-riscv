#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NCHILD 3
#define TEST_TICKS 30

struct result {
  int id;
  int work;
};

void
do_work(void)
{
  volatile uint value;
  int i;

  value = 1;

  for(i = 0; i < 20000; i++)
    value = value * 1664525 + i;
}

void
run_child(int id, int ready[2], int start[2], int result_pipe[2])
{
  struct result result;
  char value;
  int start_tick;
  int work;

  close(ready[0]);
  close(start[1]);
  close(result_pipe[0]);

  value = 1;

  if(write(ready[1], &value, 1) != 1)
    exit(1);

  close(ready[1]);

  if(read(start[0], &value, 1) != 1)
    exit(1);

  close(start[0]);

  start_tick = uptime();
  work = 0;

  while(uptime() - start_tick < TEST_TICKS){
    do_work();
    work++;
  }

  result.id = id;
  result.work = work;

  if(write(result_pipe[1], &result, sizeof(result)) !=
     (int)sizeof(result))
    exit(1);

  close(result_pipe[1]);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int ready[2];
  int start[2];
  int result_pipe[2];
  int pids[NCHILD];
  int work[NCHILD];
  struct result result;
  char values[NCHILD];
  char value;
  int minimum;
  int maximum;
  int passed;
  int i;

  passed = 1;

  for(i = 0; i < NCHILD; i++){
    work[i] = 0;
    values[i] = 1;
  }

  printf("test_prio_rr: starting\n");

  if(pipe(ready) < 0 ||
     pipe(start) < 0 ||
     pipe(result_pipe) < 0){
    printf("pipe creation failed\n");
    exit(1);
  }

  for(i = 0; i < NCHILD; i++){
    pids[i] = fork();

    if(pids[i] < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pids[i] == 0)
      run_child(i + 1, ready, start, result_pipe);
  }

  close(ready[1]);
  close(start[0]);
  close(result_pipe[1]);

  for(i = 0; i < NCHILD; i++){
    if(read(ready[0], &value, 1) != 1){
      printf("child setup failed\n");
      exit(1);
    }
  }

  close(ready[0]);

  if(setpriority(getpid(), 0) < 0){
    printf("parent priority setup failed\n");
    exit(1);
  }

  for(i = 0; i < NCHILD; i++){
    if(setpriority(pids[i], 30) < 0){
      printf("setting equal priorities failed\n");
      exit(1);
    }
  }

  if(write(start[1], values, NCHILD) != NCHILD){
    printf("starting children failed\n");
    exit(1);
  }

  close(start[1]);

  for(i = 0; i < NCHILD; i++){
    if(read(result_pipe[0], &result, sizeof(result)) !=
       (int)sizeof(result)){
      printf("reading result failed\n");
      exit(1);
    }

    if(result.id < 1 || result.id > NCHILD){
      printf("invalid child result\n");
      exit(1);
    }

    work[result.id - 1] = result.work;
  }

  close(result_pipe[0]);

  for(i = 0; i < NCHILD; i++)
    wait(0);

  printf("work counts: %d %d %d\n",
         work[0], work[1], work[2]);

  minimum = work[0];
  maximum = work[0];

  for(i = 1; i < NCHILD; i++){
    if(work[i] < minimum)
      minimum = work[i];

    if(work[i] > maximum)
      maximum = work[i];
  }

  if(minimum <= 0){
    printf("one or more processes received no CPU time\n");
    passed = 0;
  }

  if(maximum > minimum + minimum / 2 + 2){
    printf("equal priority fairness test failed\n");
    passed = 0;
  } else {
    printf("equal priority fairness test passed\n");
  }

  if(passed){
    printf("test_prio_rr: all tests passed\n");
    exit(0);
  }

  printf("test_prio_rr: tests failed\n");
  exit(1);
}
