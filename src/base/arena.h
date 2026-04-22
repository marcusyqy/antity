#ifndef _MB_ARENA_H_
#define _MB_ARENA_H_

#include <stdint.h>
#include "base.h"

typedef struct {
  void   *buf;
  size_t size;
} mb_MemoryBlock;

typedef struct {
  mb_MemoryBlock block;
  size_t         offset;
  size_t         reserved_size;
  size_t         hard_point;
} mb_Arena;

typedef struct {
  size_t count;
  b8     zero;
} mb_ArenaPushParams;

mb_Arena *mb_arena_create(size_t size, size_t reserve_size);
void      mb_arena_destroy(mb_Arena *arena);

void *mb_arena_push_raw(mb_Arena *arena, size_t size, size_t alignment, const mb_ArenaPushParams *params);
void  mb_arena_pop_to(mb_Arena *arena, size_t offset);
void  mb_arena_clear(mb_Arena *arena);

typedef struct mb_TempArena {
  mb_Arena *arena;
  size_t offset;
} mb_TempArena;

mb_Arena *mb_get_scratch_arena(mb_Arena *arena);
void      mb_scratch_clear_all(void);

mb_TempArena mb_begin_temp_arena(mb_Arena *arena);
void         mb_end_temp_arena(mb_TempArena *temp);

#define mb_arena_push(a, T, ...)      ((T *) mb_arena_push_raw((a), sizeof(T), AlignOf(T), &(mb_ArenaPushParams){ __VA_ARGS__}))
#define mb_arena_push_zero(a, T, ...) ((T *) mb_arena_push_raw((a), sizeof(T), AlignOf(T), &(mb_ArenaPushParams){ zero = true, __VA_ARGS__}))

// printf stuff
#define MB_ARENA_INFO_FMT "{block=%zu, offset=%zu, reserved=%zu}"
#define MB_ARENA_INFO_VAR(a) (a).block.size, (a).offset, (a).reserved_size


#endif // _MB_ARENA_H_
