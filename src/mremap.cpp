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
		// mremap of existing mapping needs continious map
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);
		munmap(code+4096, 4096);
		mmap(code+4096, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANON, 0, 0);

		auto code2 = (char*) mremap(code, 8192, 8192*2, MREMAP_MAYMOVE);
		printf("mmap+mmap + resize: %p, %p, pass: %d\n", code, code2, code != MAP_FAILED && code2 == MAP_FAILED);
	}

	{
		// mremap of existing mapping needs continious map
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);
		mprotect(code+4096, 4096, PROT_READ);

		auto code2 = (char*) mremap(code, 8192, 8192*2, MREMAP_MAYMOVE);
		printf("mmap+mprotect + resize: %p, %p, pass: %d\n", code, code2, code != MAP_FAILED && code2 == MAP_FAILED);
	}

	{
		// mremap of existing mapping needs continious map
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);
		mprotect(code+4096, 4096, PROT_READ);
		mprotect(code+4096, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);

		auto code2 = (char*) mremap(code, 8192, 8192*2, MREMAP_MAYMOVE);
		printf("mmap+mprotect+mprotect + resize: %p, %p, pass: %d\n", code, code2, code != MAP_FAILED && code2 != MAP_FAILED);
	}

	{
		// mremap middle part of mapping
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);

		auto code2 = (char*) mremap(code+4096, 4096, 8192*2, MREMAP_MAYMOVE);
		printf("mmap + resize offset 4096: %p, %p, pass: %d\n", code, code2, code != MAP_FAILED && code2 != MAP_FAILED);
	}

	{
		// mremap with mirror uses first page only
		auto code = (char*) mmap(0, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, 0, 0);
		munmap(code+4096, 4096);
		mmap(code+4096, 4096, PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);

		auto code2 = (char*) mremap(code, 0, 8192*2, MREMAP_MAYMOVE);
		code[0] = 192;
		code2[4096]=193;
		bool ok = code[0] == code2[0] && code[4096] != code2[4096];
		printf("mmap+mmap + mirror: %p, %p, pass: %d\n", code, code2, code != MAP_FAILED && code2 != MAP_FAILED && ok);
	}

	return 0;
}
