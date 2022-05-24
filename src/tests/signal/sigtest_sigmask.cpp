#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <ucontext.h>

volatile bool loop = true;
volatile bool inhandler = false;

#define NUMCOUNT 10
#define SIGN SIGTSTP

void sig_handler(int signum, siginfo_t *info, void *context) {
  printf("Inside handler function\n");
  if (inhandler) {
     printf("Signal reentering bug\n");
     exit(-1);
  }
  inhandler = true;
  loop = false;
  raise(signum);

  auto uctx = (ucontext_t*)context;
  sigfillset(&uctx->uc_sigmask);
}

int main() {
  struct sigaction act = { 0 };

  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = &sig_handler;
  if (sigaction(SIGN, &act, NULL) == -1) {
    exit(-2);
  }
  while (loop) {
    printf("Inside main loop, raising signal in a bit\n");
    sleep(1);
    printf("Inside main loop, raising signal\n");
    raise(SIGN);
  }
  printf("Exiting\n");
  return 0;
}
