#include <sys/mman.h>
#include <cstdio>
#include <cstdint>
#include <signal.h>
#include <string.h>

void sighandler(int signum, siginfo_t *siginfo, void *context)
{
	printf("SIG: %d, si_addr: %p\n", signum, siginfo->si_addr);
	mprotect(siginfo->si_addr, 4096, PROT_READ | PROT_WRITE);
}

int main() {

	struct sigaction sa;
	memset(&sa, 0x00, sizeof(sa));

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sighandler;
	sigaction(SIGSEGV, &sa, NULL);

	auto data = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, 0, 0);
	printf("page one: %p, page two: %p\n", data, data+4096);
	mprotect(data + 4096, 4096, PROT_NONE);
	*(int*)(data + 4094) = 0x12345678;
	return 0;
}
