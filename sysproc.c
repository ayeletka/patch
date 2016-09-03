#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

//extern sighandler_t signal(int signum, sighandler_t handler);
//extern int sigsend(int pid, int signum);
//extern int sigreturn(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int
sys_schedp(void)
{
  int policy_id;

  if(argint(0, &policy_id) < 0) {
    cprintf("sys_schedp: policy is %d", policy_id);
    return -1;
  }
  return schedp(policy_id);
}

int
sys_wait_stat(void)
{
  int* status;
  struct perf* perf_;
  if (argptr(0, (char**)&status, sizeof(int*)) < 0 ||
    argptr(1, (char**)&perf_, sizeof(struct perf*)) < 0)
  {
    return -1;
  }
  return wait_stat(status,perf_);

}

void
sys_priority(void)
{
  int priority_;


  if(argint(0, &priority_) < 0) {
    cprintf("sys_priority priority is %d", priority_);
        //return -1;
  }
  priority(priority_);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{

  int status;

  if(argint(0, &status) < 0)
    return -1;
  exit(status);
  return 0;  // not reached
}

int
sys_wait(void)
{

  int* status;

  if(argptr(0, (char**)&status, sizeof(int*)) < 0)
    return -1;
  return wait(status);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//for task3 +++
int//change to singalhandler return value?
sys_signal(void)
{
  int signum;
  sighandler_t handler = 0;

  if(argint(0, &signum) < 0)
    return -1;

  if(argptr(1, (char**)&handler,sizeof(sighandler_t)) < 0)
    return -1;
  
  if(signum < 0 || signum > NUMSIG-1)
    return -1;

  return (int)signal(signum, handler);
}
//for task3 +++

//for task3 +++
int
sys_sigsend(void)
{
  int pid;
  int signum;

  if(argint(0, &pid) < 0)
    return -1;

  if(argint(1,&signum) < 0)
    return -1;

  if(signum < 0 || signum > NUMSIG-1)
    return -1;

  return sigsend(pid, signum);
}
//for task3 +++

//for task3 +++
int sys_sigreturn(void)
{
  return sigreturn();
}
//for task3 +++