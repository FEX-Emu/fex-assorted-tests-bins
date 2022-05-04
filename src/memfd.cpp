#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void test() {
	int fd = memfd_create("/bin/sudo", 0);
	printf("pid: %d, fd: %d\n", getpid(), fd);

	struct stat st;
	fstat(fd, &st);
	printf("dev: %lu, ino: %lu\n", st.st_dev, st.st_ino);

	char syml[512];
	char temp[512];
	sprintf(syml, "/proc/%d/fd/%d", getpid(), fd);
	auto rv = readlink(syml, temp, 511);
	temp[rv] = 0;
	printf("file: %s\n", temp);
}

void mmaptest() {
	auto ptr = (char*)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0);
	auto ptr2 = (char*)mremap(ptr, 0, 8192, MREMAP_MAYMOVE);
	auto ptr3 = (char*)mremap(ptr, 4096, 8192, MREMAP_MAYMOVE);
	ptr3[0] = 1;
	ptr2[1] = 1;
	printf("%p, %p, %p\n", ptr, ptr2, ptr3);
	printf("Expecting bus error\n");
	ptr2[4096] = 1; // BUS_ERROR
	ptr3[4096] = 1; // BUS_ERROR
	//printf("%d, %d, %d, %d\n", ptr2[0], ptr3[0], ptr2[0], ptr3[0]);
}

int main() {
	test();
	mmaptest();
}
