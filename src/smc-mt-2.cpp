#include <sys/mman.h>
#include <pthread.h>
#include <cstdio>
#include <unistd.h>

char* code;

void* thread(void*) {
	printf("Generating code on thread\n");
	code[0] = 0xB8;
	code[1] = 0xAA;
	code[2] = 0xBB;
	code[3] = 0xCC;
	code[4] = 0xDD;

	code[5] = 0xC3;

	auto fn = (int(*)())code;

	fn();

	printf("Waiting for code to be modified\n");

	while (fn() == 0xDDCCBBAA) {
	}

	printf("Thread exiting\n");

	return 0;
}

int main() {
	code = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, 0, 0);

	pthread_t tid;
	pthread_create(&tid, 0, &thread, 0);

	sleep(1);

	printf("Modifying code from another thread\n");

	code[3]=0xFE;

	printf("Waiting for thread to exit\n");

	sleep(1);

	printf("Should exit now\n");
	void* rv;
	pthread_join(tid, &rv);

	return 0;
}
