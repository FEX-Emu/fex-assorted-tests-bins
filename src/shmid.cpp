#include <sys/mman.h>
#include <cstdio>
#include <cstdint>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int main() {
	auto shm = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0777);
	auto shm2 = shmget(1, 4096, IPC_CREAT | 0777);
	auto shm3 = shmget(1, 4096, IPC_CREAT | 0777);
	auto shm4 = shmget(1, 4096, 0777);
	auto shm5 = shmget(1, 8192, IPC_CREAT | 0777);

	printf("%d, %d, %d, %d, %d\n", shm, shm2, shm3, shm4, shm5);
	return 0;
}
