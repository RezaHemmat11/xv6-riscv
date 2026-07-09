#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pinfo.h"

int
main(int argc, char *argv[])
{
  struct pinfo info;

  if (getpinfo(&info) < 0) {
    printf("getpinfo failed\n");
    exit(1);
  }

  printf("PID\tSTATE\tPRIO\tTICKETS\n");

  for(int i = 0; i < 64; i++){
    if(info.state[i] != 0){
      printf("%d\t%d\t%d\t%d\n", info.pid[i], info.state[i], info.priority[i], info.tickets[i]);
    }
  }

  exit(0);
}
