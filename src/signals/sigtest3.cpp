#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <ucontext.h>

volatile int *t = (int*)0x9;

int test;
void handler (int signum, siginfo_t *si, void *old_context) {
	printf("signaled called %p\n", old_context);
	t = &test;
	((ucontext_t*)old_context)->uc_mcontext.gregs[REG_RAX] = (long)&test;
}

int main(int argc, const char **argv) {

	struct sigaction sigact;
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_sigaction = handler;

	sigact.sa_flags = SA_SIGINFO;

	printf("About to sigact\n");

	sigaction(SIGSEGV, &sigact, NULL);

	printf("About to sigsegv\n");
	*t = 32;
	printf("After sigsegv\n");

	return 0;
}
