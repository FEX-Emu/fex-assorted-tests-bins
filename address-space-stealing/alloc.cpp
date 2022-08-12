//
// cc alloc.cpp -Wl,-esteal -static -g -fno-stack-protector
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <algorithm>

#define STEAL_LOG(...)
constexpr auto BucketEntries = 32;
struct MemoryRegionBucket {
    uintptr_t PtrSize[BucketEntries + 1];
};

int main(int argc, const char **argv) {
  return printf("%s: %d\n", argv[0], argc);
}

extern "C" void _start();

void AfterGrab(MemoryRegionBucket *Root) {
  _start();
}

void StealMemoryRegion(uintptr_t Begin, uintptr_t End) {
    auto Root = (MemoryRegionBucket*)alloca(sizeof(MemoryRegionBucket));
    auto Current = Root;
    auto Position = 0;

    int MapsFD = syscall(SYS_open, "/proc/self/maps", O_RDONLY);

    enum {ParseBegin, ParseEnd, ScanEnd} State = ParseBegin;

    uintptr_t RegionBegin = 0;
    uintptr_t RegionEnd = 0;

    char Buffer[2048];
    const char *Cursor;
    ssize_t Remaining = 0;

    for(;;) {

      if (Remaining == 0) {
        do {
          Remaining = syscall(SYS_read, MapsFD, Buffer, sizeof(Buffer));
        } while ( Remaining == -1 && errno == EAGAIN);
        Cursor = Buffer;
        syscall(SYS_write, 2, Buffer, Remaining);
      }

      if (Remaining == 0 && State == ParseBegin) {
        STEAL_LOG("[%d] EndOfFile; RegionBegin: %016lX RegionEnd: %016lX\n", __LINE__, RegionBegin, RegionEnd);

        auto MapBegin = std::max(RegionEnd, Begin);
        auto MapEnd = End;

        STEAL_LOG("     MapBegin: %016lX MapEnd: %016lX\n", MapBegin, MapEnd);

        if (MapEnd > MapBegin) {
          STEAL_LOG("     Reserving\n");

          auto MapSize = MapEnd - MapBegin;
          auto Alloc = syscall(SYS_mmap, (void*)MapBegin, MapSize, PROT_NONE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE | MAP_FIXED, -1, 0);

          if (Position == (BucketEntries * 2)) {
            Current = (MemoryRegionBucket*)(Current->PtrSize[BucketEntries * 2 + 1] = (uintptr_t)alloca(sizeof(MemoryRegionBucket)));
            Position = 0;
          }
          Current->PtrSize[Position++] = MapBegin;
          Current->PtrSize[Position++] = MapSize;
          Current->PtrSize[Position] = 0;
        }

        syscall(SYS_close, MapsFD);
        AfterGrab(Root);
        return;
      }

      auto c = *Cursor++;
      Remaining--;

      if (State == ScanEnd) {
        if (c == '\n') {
          State = ParseBegin;
        }
        continue;
      }

      if (State == ParseBegin) {
        if (c == '-') {
          STEAL_LOG("[%d] ParseBegin; RegionBegin: %016lX RegionEnd: %016lX\n", __LINE__, RegionBegin, RegionEnd);

          auto MapBegin = std::max(RegionEnd, Begin);
          auto MapEnd = std::min(RegionBegin, End);

          STEAL_LOG("     MapBegin: %016lX MapEnd: %016lX\n", MapBegin, MapEnd);

          if (MapEnd > MapBegin) {
            STEAL_LOG("     Reserving\n");

            auto MapSize = MapEnd - MapBegin;
            auto Alloc = syscall(SYS_mmap, (void*)MapBegin, MapSize, PROT_NONE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE | MAP_FIXED, -1, 0);

	    if (Position == (BucketEntries * 2)) {
              Current = (MemoryRegionBucket*)(Current->PtrSize[BucketEntries * 2 + 1] = (uintptr_t)alloca(sizeof(MemoryRegionBucket)));
              Position = 0;
            }
            Current->PtrSize[Position++] = MapBegin;
            Current->PtrSize[Position++] = MapSize;
            Current->PtrSize[Position] = 0;
          }
          RegionBegin = 0;
          RegionEnd = 0;
          State = ParseEnd;
          continue;
        } else {
          RegionBegin = (RegionBegin << 4) | (c <= '9' ? (c - '0') : (c - 'a' + 10));
        }
      }

      if (State == ParseEnd) {
        if (c == ' ') {
          STEAL_LOG("[%d] ParseEnd; RegionBegin: %016lX RegionEnd: %016lX\n", __LINE__, RegionBegin, RegionEnd);

          State = ScanEnd;
          continue;
        } else {
          RegionEnd = (RegionEnd << 4) | (c <= '9' ? (c - '0') : (c - 'a' + 10));
        }
      }
    }

  }

extern "C" void steal() {
StealMemoryRegion(1 << 16, 0x800000000000 - 4 * 1024);
}
