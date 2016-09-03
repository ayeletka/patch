#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"


// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

//task3 +++
void
save_tf()
{
  proc->backup_tf.edi = proc->tf->edi;
  proc->backup_tf.esi = proc->tf->esi;
  proc->backup_tf.ebp = proc->tf->ebp;
  proc->backup_tf.oesp = proc->tf->oesp;
  proc->backup_tf.ebx = proc->tf->ebx;
  proc->backup_tf.edx = proc->tf->edx;
  proc->backup_tf.ecx = proc->tf->ecx;
  proc->backup_tf.eax = proc->tf->eax;
  proc->backup_tf.gs = proc->tf->gs;
  proc->backup_tf.padding1 = proc->tf->padding1;
  proc->backup_tf.fs = proc->tf->fs;
  proc->backup_tf.padding2 = proc->tf->padding2;
  proc->backup_tf.es = proc->tf->es;
  proc->backup_tf.padding3 = proc->tf->padding3;
  proc->backup_tf.ds = proc->tf->ds;
  proc->backup_tf.padding4 = proc->tf->padding4;
  proc->backup_tf.trapno = proc->tf->trapno;
  proc->backup_tf.err = proc->tf->err;
  proc->backup_tf.eip = proc->tf->eip;
  proc->backup_tf.cs = proc->tf->cs;
  proc->backup_tf.padding5 = proc->tf->padding5;
  proc->backup_tf.eflags = proc->tf->eflags;
  proc->backup_tf.esp = proc->tf->esp;
  proc->backup_tf.ss = proc->tf->ss;
  proc->backup_tf.padding6 = proc->tf->padding6;
}
//task3 +++

//for task3 +++
void default_handler(int num)
{
  cprintf("default_handler: A signal %d was accepted by process %d\n",num, proc->pid); 
  proc->handling_signal = 0;
}
//for task3 +++

//for task3 +++
void do_signal(struct trapframe *tf)
{
  int i;
  int check;
  int signum;
  sighandler_t curr_handler;

  if(proc && (((tf->cs) &3) == DPL_USER) && proc->pending && !(proc->handling_signal))
  {
    proc->handling_signal = 1;
  }

  else
  {
    return;
  }

  for (i = 0; i < 32; ++i)
  {
    check = (1 << i);
    check = proc->pending & check;
    if (check)
    {
      signum = i;
      proc->pending = proc->pending & (~(1<<signum));
      break;
    }
  }

  curr_handler = proc->handlers_array[signum];

  if (curr_handler == (sighandler_t)0xffffffff)
  {
    default_handler(signum);
    return; 
  }
  save_tf();
  proc->tf->esp -= 4;
  memmove((void*)proc->tf->esp,&signum,4);
  proc->tf->esp = proc->tf->esp - 4;
  memmove((void*)proc->tf->esp,&proc->ret_add,4);
  proc->tf->eip = (uint)proc->handlers_array[signum]; 

}
//for task3 +++

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}




//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit(0);
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit(0);
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      updatePerformanceFields(); //++++++++++++++++++++++
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit(0);

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER){
    yield();
  }

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit(0);
}
