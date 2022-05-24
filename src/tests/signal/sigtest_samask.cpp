#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

volatile bool loop = true;
volatile int count = 0;
volatile int count2 = 0;

#define NUMCOUNT 10
#define SIGN SIGTSTP

void sig_handler(int signum) {
  printf("Inside handler function\n");
  if (count2 != count) {
     printf("Signal reentering bug\n");
     exit(-1);
  }
  loop = false;
  if (count < NUMCOUNT) {
	printf("Nested Raising %d, %d of %d times\n", signum, 1 + count, NUMCOUNT);
	count2++;
	raise(signum);
	count++;
  }
}

int main() {
  if (signal(SIGN, sig_handler) != 0) {
    printf("Signal() failed\n");
    return -2;
  }
  while (loop) {
    printf("Inside main loop, raising signal\n");
    raise(SIGN);
  }
  printf("Exiting\n");
  return 0;
}
