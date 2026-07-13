#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int priority;

  if(argc != 3){
    printf("Usage: chpri [pid] [priority]\n");
    exit(1);
  }

  pid = atoi(argv[1]);
  priority = atoi(argv[2]);

  if(pid <= 0){
    printf("Invalid pid\n");
    exit(1);
  }

  if(priority < 0 || priority > 100){
    printf("Priority must be between 0 and 100\n");
    exit(1);
  }

  if(setpriority(pid, priority) < 0){
    printf("setpriority failed\n");
    exit(1);
  }

  printf("pid %d priority changed to %d\n", pid, priority);
  exit(0);
}
