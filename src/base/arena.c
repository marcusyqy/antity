#include "arena.h"
#include "base.h"
#include <stddef.h>
#include "os.h"

mb_Arena *mb_arena_create(size_t size, size_t reserve_size) {
  assert(size <= reserve_size);
  void *buf = mb_os_reserve_memory(reserve_size);
  mb_os_commit_memory(buf, size);
  assert(buf);

  mb_Arena arena = {
    .block = (mb_MemoryBlock){ .buf = buf, .size = size },
    .reserved_size = reserve_size,
    .offset = 0,
  };

  // @TODO: should we do this? Seems excessive but helps with the API more.
  // push the arena on the allocated memory instead.
  mb_Arena *ptr = mb_arena_push(&arena, mb_Arena);
  MemoryCopy(ptr, &arena, sizeof(mb_Arena));
  return ptr;
}

void mb_arena_destroy(mb_Arena *arena) {
  assert(arena);
  mb_os_free_memory((void *)arena->block.buf, arena->reserved_size);
}

void *mb_arena_push_raw(mb_Arena *arena, size_t size, size_t alignment, const mb_ArenaPushParams *params) {
  size_t count = params && params->count ? params->count : 1;
  ptrdiff_t buf_start = (ptrdiff_t)arena->block.buf;
  ptrdiff_t ret_val = AlignForwardPow2(buf_start + arena->offset, (ptrdiff_t)alignment);
  ptrdiff_t consumed = ret_val + size * count;
  assert(consumed > buf_start);
  size_t new_offset = (size_t)(consumed - buf_start);
  if(new_offset > arena->block.size) {
    assert(arena->reserved_size > arena->block.size);
    assert(arena->reserved_size > new_offset);
    size_t new_size = arena->block.size != 0 ? (arena->block.size * 2) : 64u;
    while(new_size < new_offset) new_size *= 2;
    const size_t determined_new_size = new_size > arena->reserved_size ? arena->reserved_size : new_size;
    assert(determined_new_size >= new_offset);
    mb_os_commit_memory(arena->block.buf, determined_new_size);
    arena->block.size = determined_new_size;
  }
  arena->offset = new_offset;
  if(params && params->zero) MemoryZero(ret_val, size);
  return (void *)ret_val;
}

void mb_arena_pop_to(mb_Arena *arena, size_t offset) {
  arena->offset = offset;
}

void mb_arena_clear(mb_Arena *arena) {
  arena->offset = 0;
}

#define MB_SCRATCH_ARENA_SIZE MB(10)
static mb_Arena *_mb_arena_private_scratch_;

// @TODO: we need to figure out when we need to do conflict resolutions.
mb_Arena *mb_get_scratch_arena(void) {
  if(_mb_arena_private_scratch_ == NULL) {
    _mb_arena_private_scratch_ = mb_arena_create(0, MB_SCRATCH_ARENA_SIZE);
  }
  return _mb_arena_private_scratch_;
}

mb_TempArena mb_begin_temp_arena(mb_Arena *arena) {
  return (mb_TempArena) { .arena = arena, .offset = arena->offset };
}

void mb_end_temp_arena(mb_TempArena *temp) {
  mb_arena_pop_to(temp->arena, temp->offset);
}

