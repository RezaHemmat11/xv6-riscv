#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pinfo.h"

char *
state_name(int state)
{
  switch(state){
  case PINFO_USED:
    return "USED";
  case PINFO_SLEEPING:
    return "SLEEPING";
  case PINFO_RUNNABLE:
    return "RUNNABLE";
  case PINFO_RUNNING:
    return "RUNNING";
  case PINFO_ZOMBIE:
    return "ZOMBIE";
  default:
    return "UNKNOWN";
  }
}

int
main(void)
{
  struct pinfo info;
  int i;

  if(getpinfo(&info) < 0){
    printf("ps: getpinfo failed\n");
    exit(1);
  }

  printf("PID\tSTATE\t\tPRIORITY\tTICKETS\n");

  for(i = 0; i < NPROC; i++){
    if(info.state[i] == PINFO_UNUSED)
      continue;

    printf("%d\t%s\t\t%d\t\t%d\n",
           info.pid[i],
           state_name(info.state[i]),
           info.priority[i],
           info.tickets[i]);

