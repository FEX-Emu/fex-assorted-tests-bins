#include <assert.h>
#include <cstdio>
#include <sys/mman.h>
#include <cstdint>
#include <errno.h>
#include <unistd.h>
#include <sys/user.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#include <chrono>


// Based on discussion in https://github.com/FEX-Emu/FEX/issues/1708
int main() {

	auto start_time = std::chrono::high_resolution_clock::now();
	size_t total_len = 0;
	size_t alloc_len = 1ULL << 51;
	while (alloc_len >= PAGE_SIZE) {
		for(;;) {
			// ~0 mmap hint to reserve the entire 52-bit address space. Needed because hugetlb & mali_kbase don't respect DEFAULT_MAP_WINDOW_64.
			// Also note the lack of MAP_FIXED: let the kernel find the VM gaps on its own.
			auto result = mmap((void*)~0ULL, alloc_len, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);

			// No more free VM gaps of this size.
			if (result == MAP_FAILED) {
				if (errno != ENOMEM && errno != EPERM /* when len is too big ?*/) {
					printf("Error, errno: %d\n", errno);
					return -1;
				} else {
					break;
				}
			}
			total_len += alloc_len;
		}

		// Halve the size and continue filling in gaps.
		alloc_len >>= 1;
	}

	std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start_time;

	printf("done, allocated %ld in %.3f ms\n", total_len, duration.count());
	return 0;
}
