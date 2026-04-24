#include "os.h"
#include "base.h"

#if OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

// @TODO: figure out whether this should be like this or not.
static struct {
  LARGE_INTEGER frequency;
  b8 initialized;
} os_state = {0};

void mb_os_init(void) {
  QueryPerformanceFrequency(&os_state.frequency);
  os_state.initialized = 1;
}

uint64_t mb_os_now_microseconds(void) {
  assert(os_state.initialized);
  uint64_t result = 0;
  LARGE_INTEGER large_int_counter;
  if(QueryPerformanceCounter(&large_int_counter)) result = (large_int_counter.QuadPart*1000000)/os_state.frequency.QuadPart;
  return result;
}

void *mb_os_reserve_memory(uint64_t size) {
  return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

void mb_os_commit_memory(void *ptr, uint64_t size) {
  if(size == 0) return;

  b8 result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0;
  (void)result;
  assert(result);
}

void mb_os_uncommit_memory(void *ptr, uint64_t size) {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

void mb_os_free_memory(void *ptr, uint64_t size) {
  (void)size;
  VirtualFree(ptr, 0, MEM_RELEASE);
}

void *mb_os_reserve_memory_large(uint64_t size) {
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_LARGE_PAGES | MEM_COMMIT, PAGE_READWRITE);
}

void mb_os_commit_memory_large(void *ptr, uint64_t size) {
  if(size == 0) return;
  (void)ptr;
  (void)size;
  // we commit and reserve when reserving.
}



#elif OS_APPLE || OS_LINUX

// @TODO: test this.

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>

#include <sys/mman.h>
#if OS_APPLE
#include <mach/vm_statistics.h>
#define HUGETBL_FLAG VM_FLAGS_SUPERPAGE_SIZE_ANY
#else
#define HUGETBL_FLAG MAP_HUGE MAP_HUGETLB
#endif

void mb_os_init(void) {
  // do nothing
}

uint64_t mb_os_now_microseconds() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  uint64_t result = t.tv_sec*1'000'000 + (t.tv_nsec/1'000);
  return result;
}

void *mb_os_reserve_memory(uint64_t size) {
  void *mem = mmap(nullptr, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if(mem == MAP_FAILED) { return nullptr; }
  return mem;
}

void mb_os_commit_memory(void *ptr, uint64_t size) {
  mprotect(ptr, size, PROT_WRITE | PROT_READ);
}

void mb_os_uncommit_memory(void *ptr, uint64_t size) {
  // tell the os we don't need this memory anymore.
  madvise(ptr, size, MADV_DONTNEED);
  // remove read/write from this region.
  mprotect(ptr, size, PROT_NONE);
}

void mb_os_free_memory(void *ptr, uint64_t size) {
  munmap(ptr, size);
}

#else
#error platform not supported
#endif // platform

