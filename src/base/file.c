#include "base.h"
#include "file.h"
#include <stdio.h>
#include "arena.h"
// #include "os.h"

// @TODO: migrate all these to OS based functions.
mb_File mb_open_file(const mb_StringView *str, mb_FileOpenMode open_mode) {
  mb_File fptr = NULL;
  mb_Arena *scratch = mb_get_scratch_arena();
  const char *file = mb_str_to_cstr(scratch, str);
  char param[3] = {
    ((open_mode & mb_FileOpenMode_Write) == mb_FileOpenMode_Write) ? 'w' : 'r',
    ((open_mode & mb_FileOpenMode_Text) == mb_FileOpenMode_Text) ? 0 : 'b' };
#if OS_WINDOWS
  fopen_s(&fptr, file, param);
#else
  fptr = fopen(file, param);
#endif
  return fptr;
}

void mb_close_file(mb_File file) {
  assert(file);
  fclose(file);
}

mb_StringView mb_file_read_bytes(mb_Arena *arena, mb_File file) {
  assert(arena);
  fseek(file, 0L, SEEK_END);
  size_t file_length = ftell(file);
  rewind(file);

  char *data = mb_arena_push(arena, char, .count = file_length);
  size_t result = fread(data, sizeof(char), file_length, file);
  assert(result == file_length);
  return (mb_StringView) {
    .data = data,
    .count = file_length,
  };
}


mb_FileNames mb_glob(mb_Arena *arena, mb_StringView pattern) {
  (void)arena;
  (void)pattern;
  // @TODO: Not done yet.
  mb_MarkNotImplemented("mb_glob: Not implemented yet");
  return (mb_FileNames){0};
}

