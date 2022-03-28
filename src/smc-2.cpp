#include <sys/mman.h>
#include <cstdio>
#include <cstdint>
#include <sys/shm.h>

void test(char* code, const char* name) {
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

	printf("%s-1: %X, %s\n", name, e1, e1 != 0xDDCCBBAA? "FAIL" : "PASS");
	printf("%s-2: %X, %s\n", name, e2, e2 != 0xDDFEBBAA? "FAIL" : "PASS");
	printf("%s-3: %X, %s\n", name, e3, e3 != 0xDDF3BBAA? "FAIL" : "PASS");
	printf("%s-4: %X, %s\n", name, e4, e4 != 0xDDF1BBAA? "FAIL" : "PASS");
}

int main() {

	auto code = (char*) mmap(0, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);

	auto code2 = (char*) mremap(code, 0, 4096, MREMAP_MAYMOVE);

	test(code2, "mremap");

	auto shm = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0777);
	auto code3 = (char*)shmat(shm, nullptr, SHM_EXEC);

	test(code3, "shmat");

	auto code4 = (char*) mremap(code3, 0, 4096, MREMAP_MAYMOVE);

	test(code4, "shmat+mremap");

	return 0;
}
