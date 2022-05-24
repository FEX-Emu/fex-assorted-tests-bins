#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>

volatile int signaled = 0;

void handler (int signum) {
	printf("signaled called\n");
	signaled = 1;
}

int main(int argc, const char **argv) {

	struct sigaction sigact;
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = handler;

	if (argc > 1) {
		sigact.sa_flags = SA_RESTART;
		printf("waiting with SA_RESTART\n");
	} else {
		printf("Waiting without SA_RESTART\n");
	}

	sigaction(SIGTSTP, &sigact, NULL);

	char ch;
	while (read(STDIN_FILENO, &ch, 1) != 1 && !signaled);

	return 0;
}
