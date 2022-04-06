#include <sys/mman.h>
#include <pthread.h>
#include <cstdio>
#include <atomic>
#include <unistd.h>
#include <signal.h>

std::atomic<bool> failed;
std::atomic<long> count;
std::atomic<long> sigcount;

int signals[] = { SIGINT, SIGSEGV, 63, SIGUSR1, SIGUSR2 };

pthread_t mainthread;

void smc_test() {
	auto code = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, 0, 0);

	for(int k = 0; k<10; k++) {
		code[0] = 0xB8;
		code[1] = 0xAA;
		code[2] = 0xBB;
		code[3] = 0xCC;
		code[4] = 0xDD;

		code[5] = 0xC3;

		auto fn = (int(*)())code;
		auto e1 = fn();
		code[3]=0xFE;
		auto e2 = fn();

		mprotect(code, 4096, PROT_READ | PROT_EXEC);

		mprotect(code, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);

		code[3] = 0xF3;

		mprotect(code, 4096, PROT_READ | PROT_EXEC);

		auto e3 = fn();

		mprotect(code, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);

		code[3] = 0xF1;

		auto e4 = fn();
		if (e1 != 0xDDCCBBAA || e2 != 0xDDFEBBAA || e3 != 0xDDF3BBAA || e4 != 0xDDF1BBAA) {
			failed = true;
		}
		count++;
		usleep(10);
	}
	munmap(code, 4096);
}

void sig_handler(int signo)
{
	sigcount++;
	smc_test();
}

void*  thread(void*) {
	for (auto sig: signals) {
		printf("SIG: %d\n", sig);
		for (int i = 0; i < 1000; i++) {
			pthread_kill(mainthread, sig);
			usleep(1);
		}
	}

	printf("SIG: Mixed\n");
	for (int i = 0; i < 1000; i++) {
		pthread_kill(mainthread, signals[i % (sizeof(signals)/sizeof(signals[0]))]);
		usleep(1);
	}
	return 0;
}


int main() {
	for (int sig: signals) {
		signal(sig, sig_handler);
	}

	mainthread = pthread_self();

	pthread_t tid;
	pthread_create(&tid, 0, &thread, 0);
	void* rv;
	pthread_join(tid, &rv);

	printf("Count: %ld, SigCount: %ld, Result: %s\n", count.load(), sigcount.load(), failed? "FAILED":"PASS");
	return failed? 1 : 0;
}
