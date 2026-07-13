#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pinfo.h"
#include "user/user.h"

#define NCHILD 3
#define TEST_TICKS 300

struct lottery_result {
  int id;
  int tickets;
  int work;
};

int
find_tickets(int pid)
{
  struct pinfo info;
  int i;

  if(getpinfo(&info) < 0)
    return -1;

  for(i = 0; i < NPROC; i++){
    if(info.pid[i] == pid)
      return info.tickets[i];
  }

  return -1;
}

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
run_worker(int id, int tickets, int ready[2], int start[2],
           int result_pipe[2])
{
  struct lottery_result result;
  char value;
  int end_tick;
  int work;

  close(ready[0]);
  close(start[1]);
  close(result_pipe[0]);

  if(settickets(tickets) < 0)
    exit(1);

  value = 1;

  if(write(ready[1], &value, 1) != 1)
    exit(1);

  close(ready[1]);

  if(read(start[0], &end_tick, sizeof(end_tick)) !=
     (int)sizeof(end_tick))
    exit(1);

  close(start[0]);

  work = 0;

  while(uptime() < end_tick){
    do_work();
    work++;
  }

  result.id = id;
  result.tickets = tickets;
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
  int inherit_ready[2];
  int inherit_release[2];
  int ready[2];
  int start[2];
  int result_pipe[2];
  int pids[NCHILD];
  int ticket_values[NCHILD];
  int work[NCHILD];
  struct lottery_result result;
  char value;
  int child;
  int end_tick;
  int passed;
  int i;

  passed = 1;
  ticket_values[0] = 10;
  ticket_values[1] = 50;
  ticket_values[2] = 100;

  for(i = 0; i < NCHILD; i++)
    work[i] = 0;

  printf("test_lottery: starting\n");

  if(find_tickets(getpid()) == 1){
    printf("default tickets test passed\n");
  } else {
    printf("default tickets test failed\n");
    passed = 0;
  }

  if(settickets(0) < 0 && settickets(-1) < 0){
    printf("invalid tickets test passed\n");
  } else {
    printf("invalid tickets test failed\n");
    passed = 0;
  }

  if(settickets(25) < 0 || find_tickets(getpid()) != 25){
    printf("settickets verification failed\n");
    passed = 0;
  } else {
    printf("settickets verification passed\n");
  }

  if(settickets(37) < 0){
    printf("inheritance setup failed\n");
    exit(1);
  }

  if(pipe(inherit_ready) < 0 || pipe(inherit_release) < 0){
    printf("inheritance pipe failed\n");
    exit(1);
  }

  child = fork();

  if(child < 0){
    printf("inheritance fork failed\n");
    exit(1);
  }

  if(child == 0){
    close(inherit_ready[0]);
    close(inherit_release[1]);

    value = 1;

    if(write(inherit_ready[1], &value, 1) != 1)
      exit(1);

    close(inherit_ready[1]);

    if(read(inherit_release[0], &value, 1) != 1)
      exit(1);

    close(inherit_release[0]);
    exit(0);
  }

  close(inherit_ready[1]);
  close(inherit_release[0]);

  if(read(inherit_ready[0], &value, 1) != 1){
    printf("inheritance child setup failed\n");
    exit(1);
  }

  close(inherit_ready[0]);

  if(find_tickets(child) == 37){
    printf("ticket inheritance test passed\n");
  } else {
    printf("ticket inheritance test failed\n");
    passed = 0;
  }

  value = 1;

  if(write(inherit_release[1], &value, 1) != 1){
    printf("inheritance release failed\n");
    exit(1);
  }

  close(inherit_release[1]);
  wait(0);

  if(settickets(1) < 0){
    printf("parent ticket reset failed\n");
    exit(1);
  }

  if(pipe(ready) < 0 || pipe(start) < 0 || pipe(result_pipe) < 0){
    printf("lottery pipe failed\n");
    exit(1);
  }

  for(i = 0; i < NCHILD; i++){
    pids[i] = fork();

    if(pids[i] < 0){
      printf("lottery fork failed\n");
      exit(1);
    }

    if(pids[i] == 0)
      run_worker(i, ticket_values[i], ready, start, result_pipe);
  }

  close(ready[1]);
  close(start[0]);
  close(result_pipe[1]);

  for(i = 0; i < NCHILD; i++){
    if(read(ready[0], &value, 1) != 1){
      printf("worker setup failed\n");
      exit(1);
    }
  }

  close(ready[0]);

  for(i = 0; i < NCHILD; i++){
    if(find_tickets(pids[i]) != ticket_values[i]){
      printf("worker ticket verification failed\n");
      passed = 0;
    }
  }

  if(passed)
    printf("worker ticket verification passed\n");

  end_tick = uptime() + TEST_TICKS;

  for(i = 0; i < NCHILD; i++){
    if(write(start[1], &end_tick, sizeof(end_tick)) !=
       (int)sizeof(end_tick)){
      printf("starting workers failed\n");
      exit(1);
    }
  }

  close(start[1]);

  for(i = 0; i < NCHILD; i++){
    if(read(result_pipe[0], &result, sizeof(result)) !=
       (int)sizeof(result)){
      printf("reading lottery result failed\n");
      exit(1);
    }

    if(result.id < 0 || result.id >= NCHILD){
      printf("invalid lottery result\n");
      exit(1);
    }

    work[result.id] = result.work;
  }

  close(result_pipe[0]);

  for(i = 0; i < NCHILD; i++)
    wait(0);

  printf("work counts: 10=%d 50=%d 100=%d\n",
         work[0], work[1], work[2]);

  if(work[0] > 0){
    printf("minimum ticket process received CPU\n");
  } else {
    printf("minimum ticket process received no CPU\n");
    passed = 0;
  }

  if(work[2] > work[1] && work[1] > work[0]){
    printf("lottery distribution test passed\n");
  } else {
    printf("lottery distribution test failed\n");
    passed = 0;
  }

  if(passed){
    printf("test_lottery: all tests passed\n");
    exit(0);
  }

  printf("test_lottery: tests failed\n");
  exit(1);
}
