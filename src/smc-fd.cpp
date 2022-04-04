#include <sys/mman.h>
#include <cstdio>
#include <cstdint>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

void test(char* codeexec, int fd, const char* name) {
	char code[6];
	code[0] = 0xB8;
	code[1] = 0xAA;
	code[2] = 0xBB;
	code[3] = 0xCC;
	code[4] = 0xDD;

	code[5] = 0xC3;

	write(fd, code, 6);

	auto fn = (int(*)())codeexec;
	auto e1 = fn();
	lseek(fd, 3, SEEK_SET);
	code[0]=0xFE;
	write(fd, code, 1);
	auto e2 = fn();

	printf("%s-1: %X, %s\n", name, e1, e1 != 0xDDCCBBAA? "FAIL" : "PASS");
	printf("%s-2: %X, %s\n", name, e2, e2 != 0xDDFEBBAA? "FAIL" : "PASS");
}

int main() {
	{
		char file[] = "smc-tests.XXXXXXXX";
		int fd = mkstemp(file);
		unlink(file);
		ftruncate(fd, 4096);

		auto code = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
		test(code, fd, "mmap_shared+fd");
	}

	{
		char file[] = "smc-tests.XXXXXXXX";
		int fd = mkstemp(file);
		unlink(file);
		ftruncate(fd, 4096);

		auto code = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
		test(code, fd, "mmap_private+fd");
	}

	{
                char file[] = "smc-tests.XXXXXXXX";
                int fd = mkstemp(file);
                int fd2 = open(file, O_RDONLY);
                unlink(file);
                ftruncate(fd, 4096);

                auto code = (char*) mmap(0, 4096, PROT_READ | PROT_EXEC, MAP_SHARED, fd2, 0);
		close(fd2);
                test(code, fd, "mmap_shared+fd2");
	}
	return 0;
}
