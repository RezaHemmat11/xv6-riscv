#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid1, pid2, pid3;

  pid1 = fork();
  if (pid1 == 0) {
    setpriority(getpid(), 80);
    for (volatile int i = 0; i < 50000000; i++);
    printf("Process with priority 80 finished\n");
    exit(0);
  }

  pid2 = fork();
  if (pid2 == 0) {
    setpriority(getpid(), 20);
    for (volatile int i = 0; i < 50000000; i++);
    printf("Process with priority 20 finished\n");
    exit(0);
  }

  pid3 = fork();
  if (pid3 == 0) {
    setpriority(getpid(), 50);
    for (volatile int i = 0; i < 50000000; i++);
    printf("Process with priority 50 finished\n");
    exit(0);
  }

  wait(0);
  wait(0);
  wait(0);

  exit(0);
}
