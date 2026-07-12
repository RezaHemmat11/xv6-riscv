#ifndef PINFO_H
#define PINFO_H

#include "param.h"

#define PINFO_UNUSED 0
#define PINFO_USED 1
#define PINFO_SLEEPING 2
#define PINFO_RUNNABLE 3
#define PINFO_RUNNING 4
#define PINFO_ZOMBIE 5

struct pinfo {
  int pid[NPROC];
  int state[NPROC];
  int priority[NPROC];
  int tickets[NPROC];
};

#endif
