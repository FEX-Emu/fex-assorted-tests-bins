#include <sys/mman.h>
#include <pthread.h>
#include <cstdio>
#include <atomic>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <stdlib.h>

std::atomic<long> sigcount;

constexpr size_t CODE_SIZE = 4 * 1024 * 1024;
int codefd;
timer_t timer;

void test() {
	auto code = (char*) mmap(0, CODE_SIZE, PROT_READ | PROT_EXEC, MAP_PRIVATE, codefd, 0);
	auto fn = (void(*)())code;
	fn();
	munmap(code, CODE_SIZE);
}

void sighandler(int signo)
{
	if (sigcount++ >= 1000) {
		timer_delete(timer);
		printf("Count: %ld\n", sigcount.load());
		exit(0);
	}

	for (int i = 0; i < 10; i++) {
		test();
	}
}

void gencode(int fd) {
        ftruncate(fd, CODE_SIZE);

	auto code = (char*)mmap(0, CODE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	int i = 0;

	while (i < (CODE_SIZE - 100)) {

		// mov eax, 0xFFFFFFFE
		code[i++] = 0xB8;
	        code[i++] = 0xFF;
	        code[i++] = 0xFF;
	        code[i++] = 0xFF;
	        code[i++] = 0xFE;


		// ladd: add eax, 1
		code[i++] = 0x05;
		code[i++] = 0;
		code[i++] = 0;
		code[i++] = 0;
		code[i++] = 1;

		// jnz ladd
		code[i++] = 0x75;
		code[i++] = -7;
	}
	// ret
	code[i++] = 0xC3;
	munmap(code, CODE_SIZE);
}

int main(int argc, const char** argv) {
        char file[] = "codegen.XXXXXXXX";
        int fd = mkstemp(file);
        unlink(file);
	gencode(fd);
	codefd = fd;

	struct sigaction action;
	memset(&action, 0, sizeof (action));
	action.sa_handler = sighandler;
	action.sa_flags = SA_NODEFER;
	sigaction (SIGALRM, &action, NULL);

	timer_create (CLOCK_MONOTONIC, NULL, &timer);

	long nsec = 100000;
	long sec = 0;

	if (argc >= 2) {
		nsec = atol(argv[1]);
	}

	if (argc >= 3) {
		sec = atol(argv[2]);
	}

	itimerspec its;
    	its.it_interval.tv_sec = sec;
	its.it_interval.tv_nsec = nsec;
	its.it_value.tv_sec = sec;
	its.it_value.tv_nsec = nsec;
	timer_settime (timer, 0, &its, NULL);

	sleep(1);

        printf("Partial Count: %ld\n", sigcount.load());
	return 1;
}
