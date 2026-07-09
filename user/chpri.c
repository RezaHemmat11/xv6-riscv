#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Usage: chpri [pid] [priority]\n");
    exit(1);
  }
  
  int pid = atoi(argv[1]);
  int priority = atoi(argv[2]);
  
  if (priority < 0 || priority > 100) {
    printf("Priority must be between 0 and 100\n");
    exit(1);
  }
  
  if (setpriority(pid, priority) < 0) {
    printf("setpriority failed\n");
    exit(1);
  }
  
  exit(0);
}
