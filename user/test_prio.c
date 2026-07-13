#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pinfo.h"
#include "user/user.h"

void
do_work(int rounds)
{
  volatile uint value;
  int i;

  value = 1;

  for(i = 0; i < rounds; i++)
    value = value * 1664525 + i;
}

int
find_priority(int pid)
{
  struct pinfo info;
  int i;

  if(getpinfo(&info) < 0)
    return -1;

  for(i = 0; i < NPROC; i++){
    if(info.pid[i] == pid)
      return info.priority[i];
  }

  return -1;
}

void
order_child(int ready[2], int start[2], int done[2], int priority)
{
  char value;

  close(ready[0]);
  close(start[1]);
  close(done[0]);

  value = 1;

  if(write(ready[1], &value, 1) != 1)
    exit(1);

  close(ready[1]);

  read(start[0], &value, 1);
  close(start[0]);

  do_work(15000000);

  if(write(done[1], &priority, sizeof(priority)) != (int)sizeof(priority))
    exit(1);

  close(done[1]);
  exit(0);
}

int
run_order_test(void)
{
  int ready[2];
  int start[2];
  int done[2];
  int pids[3];
  int priorities[3];
  int order[3];
  int passed;
  char value;
  int i;

  priorities[0] = 80;
  priorities[1] = 20;
  priorities[2] = 50;
  passed = 1;

  if(pipe(ready) < 0 || pipe(start) < 0 || pipe(done) < 0){
    printf("priority test: pipe failed\n");
    return -1;
  }

  for(i = 0; i < 3; i++){
    pids[i] = fork();

    if(pids[i] < 0){
      printf("priority test: fork failed\n");
      return -1;
    }

    if(pids[i] == 0)
      order_child(ready, start, done, priorities[i]);
  }

  close(ready[1]);
  close(start[0]);
  close(done[1]);

  for(i = 0; i < 3; i++){
    if(read(ready[0], &value, 1) != 1){
      printf("priority test: child setup failed\n");
      return -1;
    }
  }

  close(ready[0]);

  for(i = 0; i < 3; i++){
    if(find_priority(pids[i]) != 50)
      passed = 0;
  }

  if(passed)
    printf("default priority test passed\n");
  else
    printf("default priority test failed\n");

  if(setpriority(pids[0], -1) < 0)
    printf("negative priority test passed\n");
  else {
    printf("negative priority test failed\n");
    passed = 0;
  }

  if(setpriority(pids[0], 101) < 0)
    printf("priority upper bound test passed\n");
  else {
    printf("priority upper bound test failed\n");
    passed = 0;
  }

  if(setpriority(99999, 20) < 0)
    printf("invalid pid test passed\n");
  else {
    printf("invalid pid test failed\n");
    passed = 0;
  }

  for(i = 0; i < 3; i++){
    if(setpriority(pids[i], priorities[i]) < 0){
      printf("setting priorities failed\n");
      return -1;
    }
  }

  for(i = 0; i < 3; i++){
    if(find_priority(pids[i]) != priorities[i])
      passed = 0;
  }

  if(passed)
    printf("priority verification passed\n");
  else
    printf("priority verification failed\n");

  close(start[1]);

  for(i = 0; i < 3; i++){
    if(read(done[0], &order[i], sizeof(order[i])) != (int)sizeof(order[i])){
      printf("priority test: reading result failed\n");
      return -1;
    }
  }

  close(done[0]);

  wait(0);
  wait(0);
  wait(0);

  printf("finish order: %d %d %d\n", order[0], order[1], order[2]);

  if(order[0] == 20 && order[1] == 50 && order[2] == 80)
    printf("priority scheduling test passed\n");
  else {
    printf("priority scheduling test failed\n");
    passed = 0;
  }

  if(passed)
    return 0;

  return -1;
}

void
aging_child(int ready[2], int start[2], int done[2], int result, int rounds)
{
  char value;

  close(ready[0]);
  close(start[1]);
  close(done[0]);

  value = 1;

  if(write(ready[1], &value, 1) != 1)
    exit(1);

  close(ready[1]);

  read(start[0], &value, 1);
  close(start[0]);

  do_work(rounds);

  if(write(done[1], &result, sizeof(result)) != (int)sizeof(result))
    exit(1);

  close(done[1]);
  exit(0);
}

int
run_aging_test(void)
{
  int ready[2];
  int start[2];
  int done[2];
  int high_pid;
  int low_pid;
  int result[2];
  char value;
  int i;

  if(pipe(ready) < 0 || pipe(start) < 0 || pipe(done) < 0){
    printf("aging test: pipe failed\n");
    return -1;
  }

  high_pid = fork();

  if(high_pid < 0){
    printf("aging test: fork failed\n");
    return -1;
  }

  if(high_pid == 0)
    aging_child(ready, start, done, 0, 200000000);

  low_pid = fork();

  if(low_pid < 0){
    printf("aging test: fork failed\n");
    return -1;
  }

  if(low_pid == 0)
    aging_child(ready, start, done, 1, 1000);

  close(ready[1]);
  close(start[0]);
  close(done[1]);

  for(i = 0; i < 2; i++){
    if(read(ready[0], &value, 1) != 1){
      printf("aging test: child setup failed\n");
      return -1;
    }
  }

  close(ready[0]);

  if(setpriority(high_pid, 0) < 0 || setpriority(low_pid, 1) < 0){
    printf("aging test: setting priorities failed\n");
    return -1;
  }

  close(start[1]);

  for(i = 0; i < 2; i++){
    if(read(done[0], &result[i], sizeof(result[i])) != (int)sizeof(result[i])){
      printf("aging test: reading result failed\n");
      return -1;
    }
  }

  close(done[0]);

  wait(0);
  wait(0);

  printf("aging order: %d %d\n", result[0], result[1]);

  if(result[0] == 1 && result[1] == 0){
    printf("aging test passed\n");
    return 0;
  }

  printf("aging test failed\n");
  return -1;
}

int
main(int argc, char *argv[])
{
  int passed;

  passed = 1;

  printf("test_prio: starting\n");

  if(run_order_test() < 0)
    passed = 0;

  if(run_aging_test() < 0)
    passed = 0;

  if(passed){
    printf("test_prio: all tests passed\n");
    exit(0);
  }

  printf("test_prio: tests failed\n");
  exit(1);
}
