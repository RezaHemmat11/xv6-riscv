#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
cpu_work(void)
{
  volatile uint value;

  value = 1;

  for(;;)
    value = value * 1664525 + 1013904223;
}

int
main(int argc, char *argv[])
{
  int pids[4];
  int failures;
  int priority;
  int i;
  int round;

  failures = 0;

  printf("test_pstress: starting\n");

  for(i = 0; i < 4; i++){
    pids[i] = fork();

    if(pids[i] < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pids[i] == 0){
      cpu_work();
      exit(0);
    }
  }

  if(setpriority(getpid(), 0) < 0){
    printf("parent priority setup failed\n");
    exit(1);
  }

  pause(10);

  for(round = 0; round < 500; round++){
    for(i = 0; i < 4; i++){
      priority = 1 + ((round * 17 + i * 23) % 100);

      if(setpriority(pids[i], priority) < 0)
        failures++;
    }
  }

  for(i = 0; i < 4; i++)
    kill(pids[i]);

  for(i = 0; i < 4; i++)
    wait(0);

  if(failures != 0){
    printf("priority changes failed: %d\n", failures);
    printf("test_pstress: tests failed\n");
    exit(1);
  }

  printf("2000 priority changes completed\n");
  printf("all worker processes terminated\n");
  printf("no crash or deadlock detected\n");
  printf("test_pstress: all tests passed\n");
  exit(0);
}
