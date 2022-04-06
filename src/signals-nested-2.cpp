#include <sys/mman.h>
#include <pthread.h>
#include <cstdio>
#include <atomic>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <stdlib.h>

std::atomic<long> sigcount;
std::atomic<bool> signest;

constexpr size_t CODE_SIZE = 4 * 1024 * 1024;
int codefd;

void test() {
	auto code = (char*) mmap(0, CODE_SIZE, PROT_READ | PROT_EXEC, MAP_PRIVATE, codefd, 0);
	auto fn = (void(*)())code;
	fn();
	munmap(code, CODE_SIZE);
}

void sighandler(int signo)
{
	printf("Signal\n");
	sigcount++;

	sigset_t new_set;
	sigset_t old_set;
	sigemptyset(&new_set);
	sigprocmask(SIG_SETMASK, &new_set, &old_set);

	for (int i = 0; i < 100; i++) {
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
	        code[i++] = 0xFE;
	        code[i++] = 0xFF;
	        code[i++] = 0xFF;
	        code[i++] = 0xFF;


		// ladd: add eax, 1
		code[i++] = 0x05;
		code[i++] = 1;
		code[i++] = 0;
		code[i++] = 0;
		code[i++] = 0;

		// jnz ladd
		code[i++] = 0x75;
		code[i++] = -7;
	}
	// ret
	code[i++] = 0xC3;
	munmap(code, CODE_SIZE);
}

pthread_t mainthread;

void* thread(void*) {
	for (;;) {
		pthread_kill(mainthread, SIGUSR1);
		usleep(100);
		if (sigcount >= 1000) {
			printf("Finished\n");
			exit(0);
		}
	}
	return 0;
}

int main(int argc, const char** argv) {
        char file[] = "codegen.XXXXXXXX";
        int fd = mkstemp(file);
        unlink(file);
	gencode(fd);
	codefd = fd;

	signal(SIGUSR1, sighandler);

	mainthread = pthread_self();
	pthread_t tid;
	pthread_create(&tid, 0, thread, 0);
	pthread_join(tid, 0);

	return 1;
}
