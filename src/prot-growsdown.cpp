#include <sys/mman.h>
#include <stdio.h>

#define MAP_GROWSUP 0x0200

int main() {
	{
	printf("R/W growsdown\n");
	auto ptr = (char*)mmap(0, 4096 * 3, PROT_READ, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(ptr+4096*2, 4096, PROT_GROWSDOWN | PROT_READ | PROT_WRITE);
	ptr[0] = 1;
	munmap(ptr, 4096*3);
	}

	{
	printf("new vma doesn't growsdown\n");
	auto ptr = (char*)mmap(0, 4096 * 3, PROT_READ | PROT_WRITE, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(ptr+4096, 4096, PROT_NONE);
	mprotect(ptr+4096*2, 4096, PROT_GROWSDOWN | PROT_READ);
	ptr[0] = 1;
	munmap(ptr, 4096*3);
	}

	{
	printf("vma merging growsdown\n");
	auto ptr = (char*)mmap(0, 4096 * 3, PROT_READ, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(ptr+4096, 4096, PROT_NONE);
	mprotect(ptr+4096, 4096, PROT_READ);
	mprotect(ptr+4096*2, 4096, PROT_GROWSDOWN | PROT_READ | PROT_WRITE);
	ptr[0] = 1;
	munmap(ptr, 4096*3);
	}

	{
	printf("code modification after growsdown\n");
	auto code = (char*)mmap(0, 4096 * 3, PROT_READ, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(code+4096*2, 4096, PROT_GROWSDOWN | PROT_READ | PROT_WRITE | PROT_EXEC);
        code[0] = 0xB8;
        code[1] = 0xAA;
        code[2] = 0xBB;
        code[3] = 0xCC;
        code[4] = 0xDD;

        code[5] = 0xC3;

	int rv1 = ((int(*)())code)();
	code[1] = 0xAB;
	int rv2 = ((int(*)())code)();
	munmap(code, 4096*3);
	puts(rv1 == 0xDDCCBBAA && rv2 == 0xDDCCBBAB ? "SMC PASS" : "SMC FAIL");
	}

	{
	printf("code modification w/ new vma + growsdown\n");
	auto code = (char*)mmap(0, 4096 * 3, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(code+4096, 4096, PROT_NONE);
	mprotect(code+4096*2, 4096, PROT_GROWSDOWN | PROT_READ);
        code[0] = 0xB8;
        code[1] = 0xAA;
        code[2] = 0xBB;
        code[3] = 0xCC;
        code[4] = 0xDD;

        code[5] = 0xC3;

	int rv1 = ((int(*)())code)();
	code[1] = 0xAB;
	int rv2 = ((int(*)())code)();
	munmap(code, 4096*3);
	puts(rv1 == 0xDDCCBBAA && rv2 == 0xDDCCBBAB ? "SMC PASS" : "SMC FAIL");
	}

	{
	printf("code modification w/ vma merging + growsdown\n");
	auto code = (char*)mmap(0, 4096 * 3, PROT_READ, MAP_GROWSDOWN | MAP_ANON | MAP_PRIVATE, 0, 0);
	mprotect(code+4096, 4096, PROT_NONE);
	mprotect(code+4096, 4096, PROT_READ);
	mprotect(code+4096*2, 4096, PROT_GROWSDOWN | PROT_READ | PROT_WRITE | PROT_EXEC);
        code[0] = 0xB8;
        code[1] = 0xAA;
        code[2] = 0xBB;
        code[3] = 0xCC;
        code[4] = 0xDD;

        code[5] = 0xC3;

	int rv1 = ((int(*)())code)();
	code[1] = 0xAB;
	int rv2 = ((int(*)())code)();
	munmap(code, 4096*3);
	puts(rv1 == 0xDDCCBBAA && rv2 == 0xDDCCBBAB ? "SMC PASS" : "SMC FAIL");
	}

	puts("DONE");
	return 0;
}
