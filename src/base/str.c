#include "str.h"
#include <string.h>

mb_StringView mb_str_from_cstr(const char *str) {
  size_t count = strlen(str);
  // should be fine to just cast to char * since we are not using it?
  return (mb_StringView) { .data = (char *)str, .count = count };
}

char *mb_str_to_cstr(mb_Arena *arena, mb_StringView str) {
  char *data = mb_arena_push(arena, char, .count = str.count + 1);
  mb_str_to_cstr_inplace(data, str);
  return data;
}

void mb_str_to_cstr_inplace(char *cstr, mb_StringView str) {
  assert(cstr);
  MemoryCopy(cstr, str.data, str.count);
  cstr[str.count] = 0;
}
