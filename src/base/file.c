#include "base.h"
#include "file.h"
#include <stdio.h>
#include "arena.h"
// #include "os.h"

// @TODO: migrate all these to OS based functions.
mb_File mb_open_file(mb_StringView str, mb_FileOpenMode open_mode) {
  mb_File fptr = NULL;
  mb_TempArena temp = mb_begin_temp_arena(0);
  const char *file = mb_str_to_cstr(temp.arena, str);
  char param[3] = {
    ((open_mode & mb_FileOpenMode_Write) == mb_FileOpenMode_Write) ? 'w' : 'r',
    ((open_mode & mb_FileOpenMode_Text) == mb_FileOpenMode_Text) ? 0 : 'b' };
#if OS_WINDOWS
  fopen_s(&fptr, file, param);
#else
  fptr = fopen(file, param);
#endif
  mb_end_temp_arena(&temp);
  return fptr;
}

void mb_close_file(mb_File file) {
  assert(file);
  fclose(file);
}

mb_StringView mb_file_read_bytes(mb_Arena *arena, mb_File file) {
  assert(arena);
  assert(file);
  fseek(file, 0L, SEEK_END);
  size_t file_length = ftell(file);
  rewind(file);

  size_t last_offset = arena->offset;
  char *data = mb_arena_push(arena, char, .count = file_length);
  size_t result = fread(data, sizeof(char), file_length, file);
  if(result != file_length) {
    // @TODO: change to trace logging.
    fprintf(stdout, "File length not the same, expected=%zu, got=%zu. Reallocing... \n", file_length, result);
    mb_arena_pop_to(arena, last_offset);
    // realloc here.
    data = mb_arena_push(arena, char, .count = result);
    file_length = result;
  }
  return (mb_StringView) {
    .data = data,
    .count = file_length,
  };
}

void mb_file_write_bytes(mb_File file, mb_StringView data) {
  // @TODO: figure out if there is an issue with LE / BE.
  fwrite(data.data, sizeof(char), data.count, file);
}


mb_FileNames mb_glob(mb_Arena *arena, mb_StringView pattern) {
  (void)arena;
  (void)pattern;
  // @TODO: Not done yet.
  mb_MarkNotImplemented("mb_glob: Not implemented yet");
  return (mb_FileNames){0};
}

