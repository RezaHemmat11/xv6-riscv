#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pinfo.h"

int
main(int argc, char *argv[])
{
  int pids[3];
  int i;

  for(i = 0; i < 3; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf("fork failed\n");
      exit(1);
    }
    if(pids[i] == 0){
      sleep(10);
      exit(0);
    }
  }

  sleep(2);

  struct pinfo info;
  if (getpinfo(&info) < 0) {
    printf("getpinfo failed\n");
    exit(1);
  }

  printf("PID\tSTATE\tPRIO\tTICKETS\n");
  
  for(i = 0; i < 64; i++){
    if(info.state[i] != 0){
      printf("%d\t%d\t%d\t%d\n", info.pid[i], info.state[i], info.priority[i], info.tickets[i]);
    }
  }

  for(i = 0; i < 3; i++){
    wait(0);
  }

  exit(0);
}
