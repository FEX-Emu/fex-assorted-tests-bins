// https://gist.github.com/DanGe42/7148946

#include <ucontext.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <stdint.h>

// Switches
// SIGACTION
// LARGESLICE
// LARGESTACK
// BUGGYSETUP
// EXTRASLEEP

/* ucontext sample program
   by Jon Kaplan and Robert Spier (October 24th, 1999)
   Updated for 2000 and poll, Robert Spier
   sigprocmask gaff fixed by Ben Slusky
   ported to Linux by Eric Cronin
   $Id: context_demo.c 37 2006-10-12 22:16:59Z ecronin $
   Demonstrates swapping between multiple processor contexts (in a
   _stable_ way).  n-1 contexts do nothing.  1 context accepts input
   and outputs it.
*/


#define NUMCONTEXTS 10              /* how many contexts to make */
#if !defined(LARGESTACK)
#define STACKSIZE (4096)              /* stack size */
#else
#define STACKSIZE (8 * 4096)              /* stack size */
#endif
#if !defined(LARGESLICE)
#define INTERVAL 1                /* timer interval in nano\seconds */
#else
#define INTERVAL 100
#endif

//sigset_t set;                       /* process wide signal mask */
ucontext_t signal_context;          /* the interrupt context */
void *signal_stack;                 /* global interrupt stack */

ucontext_t contexts[NUMCONTEXTS];   /* store our context info */
int curcontext = 0;                 /* whats the current context? */
uintptr_t contextstack[NUMCONTEXTS]; // stack pointer in the inner loop
int insig[NUMCONTEXTS];             // >0 if handling a signal
ucontext_t *cur_context;            /* a pointer to the current_context */

/* The scheduling algorithm; selects the next context to run, then starts it. */
void
scheduler()
{
//    printf("scheduling out thread %d\n", curcontext);

    curcontext = (curcontext + 1) % NUMCONTEXTS; /* round robin */
    cur_context = &contexts[curcontext];

    printf("scheduling in thread %d\n", curcontext);

    setcontext(cur_context); /* go */
}

/*
  Timer interrupt handler.
  Creates a new context to run the scheduler in, masks signals, then swaps
  contexts saving the previously executing thread and jumping to the
  scheduler.
*/
void
#if defined(SIGACTION)
timer_interrupt(int j, siginfo_t *si, void *old_context)
#else
timer_interrupt(int j)
#endif
{
    int signest = insig[curcontext]++;
    if (signest) {
        printf("Nested signals\n");
	exit(-2);
    }
#if defined(EXTRASLEEP)
    usleep(INTERVAL * 3);
#endif

    uintptr_t used = STACKSIZE - ((uintptr_t)&j -  (uintptr_t)cur_context->uc_stack.ss_sp);
    uintptr_t sigused = contextstack[curcontext] - (uintptr_t)&j;

    printf("Fiber: %d, Signal enter: stack %p, used: %ld, sigused: %ld\n", curcontext, &j, used, sigused);
    if (used >= STACKSIZE) {
	printf("Stack overflow\n");
	exit(-1);
    }
    /* Create new scheduler context */
    getcontext(&signal_context);
    signal_context.uc_stack.ss_sp = signal_stack;
    signal_context.uc_stack.ss_size = STACKSIZE;
    signal_context.uc_stack.ss_flags = 0;
    sigemptyset(&signal_context.uc_sigmask);
    makecontext(&signal_context, scheduler, 0);
    /* save running thread, jump to scheduler */
    swapcontext(cur_context,&signal_context);
#if defined(EXTRASLEEP)
    usleep(INTERVAL * 3);
#endif
    insig[curcontext]--;
}

/* Set up SIGALRM signal handler */
void
setup_signals(void)
{
#if defined(SIGACTION)
    struct sigaction act;

    act.sa_sigaction = timer_interrupt;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    int err = sigaction(SIGALRM, &act, NULL) != 0;
#else
    int err = signal(SIGALRM, &timer_interrupt) != SIG_ERR;
#endif
    if(err) {
        perror("Signal handler");
    }

#if !defined(BUGGYSETUP)
    sigset_t x;
    sigemptyset (&x);
    sigaddset(&x, SIGALRM);
    sigprocmask(SIG_BLOCK, &x, NULL);
#endif
}


/* Thread bodies */
void
thread1()
{
    char p;
    contextstack[curcontext] = (uintptr_t)&p;

    while(1) {
        poll(NULL,0,10);
	//write(1, "t1\n", 3);

    };     /* do nothing nicely */
}

void
thread2()
{
    char buf[1024];
    char p;
    contextstack[curcontext] = (uintptr_t)&p;

    /* get a string.. print a string.. ad infinitum */
    while(1) {
        poll(NULL,0,10);
	//write(1, "t2\n", 3);
    }
}

/* helper function to create a context.
   initialize the context from the current context, setup the new
   stack, signal mask, and tell it which function to call.
*/
void
mkcontext(ucontext_t *uc,  void *function)
{
    void * stack;

    getcontext(uc);

    stack = malloc(STACKSIZE);
    if (stack == NULL) {
        perror("malloc");
        exit(1);
    }
    /* we need to initialize the ucontext structure, give it a stack,
        flags, and a sigmask */
    uc->uc_stack.ss_sp = stack;
    uc->uc_stack.ss_size = STACKSIZE;
    uc->uc_stack.ss_flags = 0;
    printf("Stack: %p, %d\n", stack, STACKSIZE);
    if (sigemptyset(&uc->uc_sigmask) < 0){
      perror("sigemptyset");
      exit(1);
    }

    /* setup the function we're going to, and n-1 arguments. */
    makecontext(uc, function, 1);

}


int
main()
{
    int i;
    struct itimerval it;

    fprintf(stderr,"Process Id: %d\n", (int)getpid());

    /* allocate the global signal/interrupt stack */
    signal_stack = malloc(STACKSIZE);
    if (signal_stack == NULL) {
        perror("malloc");
        exit(1);
    }

    /* make all our contexts */
    mkcontext(&contexts[0], thread2);
    for(i=1; i < NUMCONTEXTS; i++)
        mkcontext(&contexts[i], thread1);


    /* initialize the signal handlers */
    setup_signals();

    /* setup our timer */
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = INTERVAL * 1000;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_REAL, &it, NULL) ) perror("setitiimer");

#if defined(EXTRASLEEP)
    usleep(INTERVAL * 3);
#endif

#if !defined(BUGGYSETUP)
    printf("Starting fiber execution\n");
#endif
    /* force a swap to the first context */
    cur_context = &contexts[0];
    setcontext(&contexts[0]);

    return 0; /* make gcc happy */
}
