#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pinfo.h"

int
find_process(struct pinfo *info, int pid)
{
  int i;

  for(i = 0; i < NPROC; i++){
    if(info->pid[i] == pid && info->state[i] != PINFO_UNUSED)
      return i;
  }

  return -1;
}

int
main(void)
{
  struct pinfo info;
  int children[3];
  int parent;
  int created;
  int passed;
  int pid;
  int index;
  int i;

  parent = getpid();
  created = 0;
  passed = 1;

  printf("test_pinfo: starting\n");

  for(i = 0; i < 3; i++){
    pid = fork();

    if(pid < 0){
      printf("test_pinfo: fork failed\n");

      for(i = 0; i < created; i++)
        kill(children[i]);

      for(i = 0; i < created; i++)
        wait(0);

      exit(1);
    }

    if(pid == 0){
      pause(100);
      exit(0);
    }

    children[created] = pid;
    created++;
  }

  pause(5);

  if(getpinfo(&info) < 0){
    printf("test_pinfo: getpinfo failed\n");

    for(i = 0; i < created; i++)
      kill(children[i]);

    for(i = 0; i < created; i++)
      wait(0);

    exit(1);
  }

  index = find_process(&info, parent);

  if(index < 0){
    printf("parent process not found\n");
    passed = 0;
  } else {
    printf("parent pid %d found, state %d\n",
           parent, info.state[index]);
  }

  for(i = 0; i < created; i++){
    index = find_process(&info, children[i]);

    if(index < 0){
      printf("child pid %d not found\n", children[i]);
      passed = 0;
    } else {
      printf("child pid %d found, state %d, priority %d, tickets %d\n",
             children[i],
             info.state[index],
             info.priority[index],
             info.tickets[index]);
    }
  }

  if(getpinfo((struct pinfo *)(uint64)-1) < 0){
    printf("invalid address test passed\n");
  } else {
    printf("invalid address test failed\n");
    passed = 0;
  }

  for(i = 0; i < created; i++)
    kill(children[i]);

  for(i = 0; i < created; i++)
    wait(0);

  if(passed){
    printf("test_pinfo: all tests passed\n");
    exit(0);
  }

  printf("test_pinfo: test failed\n");
  exit(1);
}
