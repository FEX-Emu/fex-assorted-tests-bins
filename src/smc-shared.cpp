#include <sys/mman.h>
#include <cstdio>
#include <cstdint>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

void test(char* code, char* codeexec, const char* name) {
	assert(code != codeexec);
	code[0] = 0xB8;
	code[1] = 0xAA;
	code[2] = 0xBB;
	code[3] = 0xCC;
	code[4] = 0xDD;

	code[5] = 0xC3;

	auto fn = (int(*)())codeexec;
	auto e1 = fn();
	code[3]=0xFE;
	auto e2 = fn();

	printf("%s-1: %X, %s\n", name, e1, e1 != 0xDDCCBBAA? "FAIL" : "PASS");
	printf("%s-2: %X, %s\n", name, e2, e2 != 0xDDFEBBAA? "FAIL" : "PASS");
}

int main() {

	{
		auto code = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);

		auto code2 = (char*) mremap(code, 0, 4096, MREMAP_MAYMOVE);

		test(code, code2, "mmap+mremap");
	}

	{
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);

		auto code2 = (char*) mremap(code+4096, 0, 4096, MREMAP_MAYMOVE);

		test(code+4096, code2, "mmap+mremap_mid");
	}

	{
		auto shm = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0777);
		auto code3 = (char*)shmat(shm, nullptr, 0);
		auto code4 = (char*)shmat(shm, nullptr, SHM_EXEC);
		test(code3, code4, "shmat");
	}

	{
		auto shm2 = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0777);
		auto code5 = (char*)shmat(shm2, nullptr, SHM_EXEC);
		auto code6 = (char*)mremap(code5, 0, 4096, MREMAP_MAYMOVE);

		test(code5, code6, "shmat+mremap");
	}

	{
		auto shm2 = shmget(IPC_PRIVATE, 8192, IPC_CREAT | 0777);
		auto code5 = (char*)shmat(shm2, nullptr, SHM_EXEC);
		auto code6 = (char*)mremap(code5+4096, 0, 4096, MREMAP_MAYMOVE);

		test(code5+4096, code6, "shmat+mremap_mid");
	}

	{
		char file[] = "smc-tests.XXXXXXXX";
		int fd = mkstemp(file);
		unlink(file);
		ftruncate(fd, 4096);

		auto code7 = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		auto code8 = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
		test(code7, code8, "mmap+mmap");
	}

	{
		char file[] = "smc-tests.XXXXXXXX";
		int fd = mkstemp(file);
		int fd2 = open(file, O_RDONLY);
		unlink(file);
		ftruncate(fd, 4096);

		auto code = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		auto code2 = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd2, 0);
		test(code, code2, "mmap+mmap (fd, fd2)");
	}

	{
		char file[] = "smc-tests.XXXXXXXX";
		mktemp(file);
		int fd = shm_open(file,O_RDWR | O_CREAT, 0700);
		shm_unlink(file);
		ftruncate(fd, 4096);

		auto code7 = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		auto code8 = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
		test(code7, code8, "shm_open+mmap+mmap");
	}

	{
		char file[] = "smc-tests.XXXXXXXX";
		mktemp(file);
		int fd = shm_open(file,O_RDWR | O_CREAT, 0700);
		int fd2 = shm_open(file, O_RDONLY, 0700);
		shm_unlink(file);
		ftruncate(fd, 4096);

		auto code7 = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		auto code8 = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd2, 0);
		test(code7, code8, "shm_open+mmap+mmap (fd, fd2)");
	}

	return 0;
}
