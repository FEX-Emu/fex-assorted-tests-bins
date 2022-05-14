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

// Based on discussion in https://github.com/FEX-Emu/FEX/issues/1708
int main() {

	size_t total_len = 0;
	size_t alloc_len = 1ULL << 51;
	while (alloc_len >= PAGE_SIZE) {
		for(;;) {
			// ~0 mmap hint to reserve the entire 52-bit address space. Needed because hugetlb & mali_kbase don't respect DEFAULT_MAP_WINDOW_64.
			// Also note the lack of MAP_FIXED: let the kernel find the VM gaps on its own.
			auto result = mmap((void*)~0ULL, alloc_len, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);

			// No more free VM gaps of this size.
			if (result == MAP_FAILED) {
				assert(errno == ENOMEM);
				break;
			}
			total_len += alloc_len;
		}

		// Halve the size and continue filling in gaps.
		alloc_len >>= 1;
	}
	printf("done, allocated %ld\n", total_len);
	return 0;
}
