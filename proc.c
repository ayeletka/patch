#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
//#include <time.h>
#include "statForSanity.h"
#include <sys/types.h>//task3 +++
static unsigned long int next = 1;

int started = 0;
struct {
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);
//int ticketCount = 1;
int currentPolicy = 1;

static void wakeup1(void *chan);

//task3 +++
void
restore_tf()
{
  proc->tf->edi = proc->backup_tf.edi;
  proc->tf->esi = proc->backup_tf.esi;
  proc->tf->ebp = proc->backup_tf.ebp;
  proc->tf->oesp = proc->backup_tf.oesp;
  proc->tf->ebx = proc->backup_tf.ebx;
  proc->tf->edx = proc->backup_tf.edx;
  proc->tf->ecx = proc->backup_tf.ecx;
  proc->tf->eax = proc->backup_tf.eax;
  proc->tf->gs = proc->backup_tf.gs;
  proc->tf->padding1 = proc->backup_tf.padding1;
  proc->tf->fs = proc->backup_tf.fs;
  proc->tf->padding2 = proc->backup_tf.padding2;
  proc->tf->es = proc->backup_tf.es;
  proc->tf->padding3 = proc->backup_tf.padding3;
  proc->tf->ds = proc->backup_tf.ds;
  proc->tf->padding4 = proc->backup_tf.padding4;
  proc->tf->trapno = proc->backup_tf.trapno;
  proc->tf->err = proc->backup_tf.err;
  proc->tf->eip = proc->backup_tf.eip;
  proc->tf->cs = proc->backup_tf.cs;
  proc->tf->padding5 = proc->backup_tf.padding5;
  proc->tf->eflags = proc->backup_tf.eflags;
  proc->tf->esp = proc->backup_tf.esp;
  proc->tf->ss = proc->backup_tf.ss;
  proc->tf->padding6 = proc->backup_tf.padding6;
}
//task3 +++

//for task3 +++
void set_default_handlers(struct proc *p)
{
	p->pending = 0;
	p->handling_signal = 0;
	int i;
	for (i = 0; i < 32; ++i)
	{
		p->handlers_array[i] = (sighandler_t)0xffffffff;
	}
}
//for task3 +++

//for task3 +++
sighandler_t signal(int signum, sighandler_t handler)
{
  sighandler_t prev;  
  if (NUMSIG-1 < signum || signum < 0)
    return (sighandler_t)-1;

  acquire(&ptable.lock);
  prev = proc->handlers_array[signum];  
  proc->handlers_array[signum] = handler;
  release(&ptable.lock);
  return prev;
}
//for task3 +++

//for task3 +++
int sigsend(int pid, int signum)
{
	if (NUMSIG-1 < signum || signum < 0)
		return -1; 

	struct proc *p;
	acquire(&ptable.lock);
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
	{
		if(p!=0 && p->pid == pid)
		{
      p->pending = p->pending | (1 << signum);
      release(&ptable.lock);
      return 0; 
    }
  }

  release(&ptable.lock);
  return -1;
}
//for task3 +++

//task3 +++
int sigreturn(void)
{
  acquire(&ptable.lock);
  proc->handling_signal = 0;
	restore_tf();
  release(&ptable.lock);
	return 0; 
}
//task3 +++


  void
  pinit(void)
  {
  	initlock(&ptable.lock, "ptable");
  }

//++++++++++++++++++++++++++++++++++++++++++++++++
  void updatePerformanceFields(){
  	acquire(&ptable.lock);
  	struct proc *p;
  	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  		if(p -> state == RUNNING){
  			p -> rutime++;

  		}
  		else if(p -> state == SLEEPING){
  			p -> stime++;
  		}
  		else if(p -> state == RUNNABLE){
  			p -> retime++;
  		}
  	}
  	release(&ptable.lock);

  }
//+++++++++++++++++++++++++++++++++++++++++++++++++

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
  static struct proc*
  allocproc(void)
  {
  	struct proc *p;
  	char *sp;

  	acquire(&ptable.lock);
  	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  		if(p->state == UNUSED)
  			goto found;
  		release(&ptable.lock);
  		return 0;

  		found:
  		p->state = EMBRYO;
  		p->pid = nextpid++;

      set_default_handlers(p);//task3 +++

      release(&ptable.lock);


  // Allocate kernel stack.
      if((p->kstack = kalloc()) == 0){
       p->state = UNUSED;
       return 0;
     }
     sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
     sp -= sizeof *p->tf;
     p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
     sp -= 4;
     *(uint*)sp = (uint)trapret;

     sp -= sizeof *p->context;
     p->context = (struct context*)sp;
     memset(p->context, 0, sizeof *p->context);
     p->context->eip = (uint)forkret;

     return p;
   }

//PAGEBREAK: 32
// Set up first user process.
   void
   userinit(void)
   {

    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();
    initproc = p;
    if((p->pgdir = setupkvm()) == 0)
     panic("userinit: out of memory?");
   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
   p->sz = PGSIZE;
   memset(p->tf, 0, sizeof(*p->tf));
   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
   p->tf->es = p->tf->ds;
   p->tf->ss = p->tf->ds;
   p->tf->eflags = FL_IF;
   p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S



  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  p->priority = 10;
  //distribute the tickets
  if(currentPolicy==1) {
    //ticketCount = ticketCount + 10;
  	p->ntickets = 10;
  	cprintf("p->ntickets from userinit (policy 1): %d\n", p->ntickets);
    //cprintf("tickets count from userinit (policy 1): %d\n", ticketCount);
  }
  else if (currentPolicy==2){
  	p->ntickets = p->priority;
    //ticketCount = ticketCount + p->ntickets;
  	cprintf("p->ntickets from userinit (policy 2): %d\n", p->ntickets);
    //cprintf("tickets count from userinit (policy 2): %d\n", ticketCount);
  }
  else if (currentPolicy==3){
  	p->ntickets = 20;
      //ticketCount = ticketCount + 20;
  	cprintf("p->ntickets from userinit (policy 3): %d\n", p->ntickets);
      //cprintf("tickets count from userinit (policy 3): %d\n", ticketCount);

  }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  p->state = RUNNABLE;
    p->ctime = ticks; //+++++++++++++++
  }

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
  int
  growproc(int n)
  {
   uint sz;

   sz = proc->sz;
   if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
     return -1;
 } else if(n < 0){
  if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
   return -1;
}
proc->sz = sz;
switchuvm(proc);
return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
	int i, pid;
	struct proc *np;

  // Allocate process.
	if((np = allocproc()) == 0)
		return -1;

  // Copy process state from p.
	if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	np->sz = proc->sz;
	np->parent = proc;
	*np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
   if(proc->ofile[i])
    np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  //np->ret_add = proc->ret_add;

  safestrcpy(np->name, proc->name, sizeof(proc->name));

  pid = np->pid;




  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  np->priority = 10;
  // distribute the tickets
  if(currentPolicy==1) {
    //ticketCount = ticketCount + 10;
    np->ntickets = 10;
    //cprintf("np->ntickets from fork (policy 1): %d\n", np->ntickets);
    //cprintf("tickets count from fork (policy 1): %d\n", ticketCount);
  }
  else if (currentPolicy==2){
    np->ntickets = np->priority;
    //ticketCount = ticketCount + np->ntickets;
    //cprintf("np->ntickets from fork (policy 2): %d\n", np->ntickets);
    //cprintf("tickets count from fork (policy 2): %d\n", ticketCount);


  }
  else if (currentPolicy==3){
    np->ntickets = 20;
      //ticketCount = ticketCount + 20;
    //cprintf("np->ntickets from fork (policy 3): %d\n", np->ntickets);
      //cprintf("tickets count from fork (policy 3): %d\n", ticketCount);

  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    np->stime = 0;   //++++++++++++++
    np->rutime = 0;   //++++++++++++++
    np->retime = 0;   //++++++++++++++
    np->state = RUNNABLE;
    np->ctime = ticks;   //++++++++++++++

    release(&ptable.lock);

    return pid;
  }

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
  void
exit(int status) //+++
{
	struct proc *p;
	int fd;

	if(proc == initproc)
		panic("init exiting");



  // Close all open files.
	for(fd = 0; fd < NOFILE; fd++){
		if(proc->ofile[fd]){
			fileclose(proc->ofile[fd]);
			proc->ofile[fd] = 0;
		}
	}

	begin_op();
	iput(proc->cwd);
	end_op();
	proc->cwd = 0;

	acquire(&ptable.lock);

  // Parent might be sleeping in wait().
	wakeup1(proc->parent);

  // Pass abandoned children to init.
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		if(p->parent == proc){
			p->parent = initproc;
			if(p->state == ZOMBIE)
				wakeup1(initproc);
		}
	}

  // Jump into the scheduler, never to return.
	proc->ttime = ticks;
	proc->state = ZOMBIE;
  //+++++++++++++++++++++
	proc->status = status;
   /* if(ticketCount > proc->ntickets) {
        cprintf("Exit called! before: ticketCount %d\n", ticketCount);

        ticketCount = ticketCount - proc->ntickets;
        cprintf("Exit called! after: ticketCount %d\n", ticketCount);
    }*/
        proc->ntickets = 0;
        //cprintf("Exit called! proc->status %d\n", proc->status); // for task 1

  //+++++++++++++++++++++

        sched();
        panic("zombie exit");
      }

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
      int
      wait(int* status)
      {
       struct proc *p;
       int havekids, pid;

       acquire(&ptable.lock);
       for(;;){
    // Scan through table looking for zombie children.
        havekids = 0;
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
         if(p->parent != proc)
          continue;
        havekids = 1;
        if(p->state == ZOMBIE){
        //+++++++++++++++++++++ Found one.
          cprintf("wait called! proc->status %d\n",p->status);

          if(status!=0)
           *status = p->status;
         cprintf("wait called! *status %d\n",(int)*status);

         p->status = 0;

         // ticketCount -= p->ntickets;
         p->ntickets = 0;
//++++++++++++++++++++++++++++++++++
         pid = p->pid;
         kfree(p->kstack);
         p->kstack = 0;
         freevm(p->pgdir);
         p->state = UNUSED;
         p->pid = 0;
         p->parent = 0;
         p->name[0] = 0;
         p->killed = 0;
         release(&ptable.lock);

         return pid;
       }
     }

    // No point waiting if we don't have any children.
     if(!havekids || proc->killed){
       release(&ptable.lock);
       return -1;
     }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}



int rand(int ticketsSum) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	int rand = (unsigned int)(next/(2 * (ticketsSum +1)) % (ticketsSum+1));
	return rand ;
}

void srand(unsigned int seed)
{
	next = seed;
}
/*
uint randomFunc (int n)
{
  static uint z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
  uint b;
  b  = ((z1 << 6) ^ z1) >> 13;
  z1 = ((z1 & 4294967294U) << 18) ^ b;
  b  = ((z2 << 2) ^ z2) >> 27;
  z2 = ((z2 & 4294967288U) << 2) ^ b;
  b  = ((z3 << 13) ^ z3) >> 21;
  z3 = ((z3 & 4294967280U) << 7) ^ b;
  b  = ((z4 << 3) ^ z4) >> 12;
  z4 = ((z4 & 4294967168U) << 13) ^ b;
  return (z1 ^ z2 ^ z3 ^ z4)%n;
}*/


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
  void
  scheduler(void)
  {
  	struct proc *p;
  	srand(ticks);
  	for(;;){
    // Enable interrupts on this processor.
  		sti();

    // Loop over process table looking for process to run.
  		acquire(&ptable.lock);

      //take 2:

  		int ticketCounterAll = 1;
  		for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
  			if(  p->state!= ZOMBIE)
  				ticketCounterAll = ticketCounterAll + p->ntickets;


  		}

      //int chosenTicket = ((ticks*ticks*ticks)-13 + ticks) %ticketCounterAll;
  		int chosenTicket = rand(ticketCounterAll);
  		if(started)
  			cprintf("Chosen %d\n", chosenTicket);
          //cprintf("PID:%d\nChosen: %d\nCounter: %d\nPticket:%d\n", p->pid, chosenTicket, ticketCounter, p->ntickets);

  		int ticketCounter = 0;
  		for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  			ticketCounter = ticketCounter + p->ntickets;
  			if(chosenTicket >= ticketCounter)
  				continue;
        //The process has the correct ticket:
  			if(p->state != RUNNABLE)
  				break;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
  			proc = p;
  			switchuvm(p);
  			p->state = RUNNING;
  			swtch(&cpu->scheduler, proc->context);
  			switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
  			proc = 0;
  		}
  		release(&ptable.lock);

  	}
  }


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
  void
  sched(void)
  {
  	int intena;

  	if(!holding(&ptable.lock))
  		panic("sched ptable.lock");
  	if(cpu->ncli != 1)
  		panic("sched locks");
  	if(proc->state == RUNNING)
  		panic("sched running");
  	if(readeflags()&FL_IF)
  		panic("sched interruptible");
  	intena = cpu->intena;
  	swtch(&proc->context, cpu->scheduler);
  	cpu->intena = intena;
  }

// Give up the CPU for one scheduling round.
  void
  yield(void)
  {
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++
  if(currentPolicy==3) {
  	if (proc->ntickets > 1){
  		proc->ntickets -= 1;
            //ticketCount-=1;
  	}
  }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++

  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
	static int first = 1;
  // Still holding ptable.lock from scheduler.
	release(&ptable.lock);

	if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
		first = 0;
		iinit(ROOTDEV);
		initlog(ROOTDEV);
	}

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
	if(proc == 0)
		panic("sleep");

	if(lk == 0)
		panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;

  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
  	release(&ptable.lock);
  	acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
	struct proc *p;

	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if(p->state == SLEEPING && p->chan == chan)
			p->state = RUNNABLE;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++

		if(currentPolicy==3) {
			if (p->ntickets < 90) {
				p->ntickets += 10;
            //ticketCount += 10;
			}
		}
    //++++++++++++++++++++++++++++++++++++++++++++++++++++

	}

// Wake up all processes sleeping on chan.
	void
	wakeup(void *chan)
	{
		acquire(&ptable.lock);
		wakeup1(chan);
		release(&ptable.lock);
	}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
	int
	kill(int pid)
	{
		struct proc *p;

		acquire(&ptable.lock);
		for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
			if(p->pid == pid){
				p->killed = 1;
        //++++++++++++++++++++++
				p->ttime = ticks;
       // if(ticketCount>p->ntickets)
       //     ticketCount -= p->ntickets;
        //p->ntickets = 0;
        //++++++++++++++++++++++
      // Wake process from sleep if necessary.
				if(p->state == SLEEPING)
					p->state = RUNNABLE;
				release(&ptable.lock);
				return 0;
			}
		}
		release(&ptable.lock);
		return -1;
	}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	int schedp(int policy_id){
		cprintf("the policy is: %d\n",policy_id);

		if(1<=policy_id && policy_id<=3){
			cprintf("the policy is: %d\n",policy_id);
			currentPolicy = policy_id;}
			else  panic("schedp: policy needs to be 1/2/3");
			return 0;

		}

		int wait_stat(int* status, struct perf* perf_){
			struct proc *p;
			int havekids, pid;

			acquire(&ptable.lock);
			for(;;){
        // Scan through table looking for zombie children.
				havekids = 0;
				for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
					if (p->parent != proc)
						continue;
					havekids = 1;
					if (p->state == ZOMBIE) {
                //+++++++++++++++++++++ Found one.

						*status = p->status;
						perf_->stime = p->stime;
						perf_->retime = p->retime;
						perf_->rutime = p->rutime;
						perf_->ttime = p->ttime;
						perf_->ctime = p->ctime;

						//cprintf("wait_stat called! *status %d\n", *status);//task2
						cprintf("wait_stat called! perf_->ctime %d\n", perf_->ctime);//task2
						cprintf("wait_stat called! perf_->ttime %d\n", perf_->ttime);//task2
              //  cprintf("wait_stat called! p->ntickets %d\n",  p->ntickets);//task2



						p->status = 0;
						p->stime = 0;
						p->retime = 0;
						p->rutime = 0;
						p->ttime = 0;
						p->ctime = 0;
						p->ntickets = 0;
						p->priority = 0;
//++++++++++++++++++++++++++++++++++

						pid = p->pid;
						kfree(p->kstack);
						p->kstack = 0;
						freevm(p->pgdir);
						p->state = UNUSED;
						p->pid = 0;
						p->parent = 0;
						p->name[0] = 0;
						p->killed = 0;
						release(&ptable.lock);

						return pid;
					}
				}

        // No point waiting if we don't have any children.
				if(!havekids || proc->killed){
					release(&ptable.lock);
					return -1;
				}

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);  //DOC: wait-sleep
      }
    }

    void priority(int priority_){
     cprintf("the priority_ is: %d\n",priority_);
     proc->priority = priority_;
     cprintf("the proc->priority is: %d\n",proc->priority);



   }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
   void
   procdump(void)
   {
     started = 1;
     static char *states[] = {
      [UNUSED]    "unused",
      [EMBRYO]    "embryo",
      [SLEEPING]  "sleep ",
      [RUNNABLE]  "runble",
      [RUNNING]   "run   ",
      [ZOMBIE]    "zombie"
    };
    int i;
    struct proc *p;
    char *state;
    uint pc[10];

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state == UNUSED)
       continue;
     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
       state = states[p->state];
     else
       state = "???";
     cprintf("%d %s %s %d", p->pid, state, p->name, p->ntickets);
     if(p->state == SLEEPING){
       getcallerpcs((uint*)p->context->ebp+2, pc);
       for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
    cprintf("retime:%d\n", p->retime);
    cprintf("rutime:%d\n", p->rutime);
    cprintf("ctime:%d\n", p->ctime);
    cprintf("ttime:%d\n", p->ttime);
    cprintf("stime:%d\n", p->stime);
  }
}
