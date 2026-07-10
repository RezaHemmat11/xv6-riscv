#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pids[3];
  int tickets[3] = {10, 50, 100};
  int i;

  for(i = 0; i < 3; i++) {
    pids[i] = fork();
    if (pids[i] == 0) {
      settickets(tickets[i]);
      for (volatile int j = 0; j < 50000000; j++);
      printf("Process with %d tickets finished\n", tickets[i]);
      exit(0);
    }
  }

  for(i = 0; i < 3; i++){
    wait(0);
  }

  exit(0);
}
