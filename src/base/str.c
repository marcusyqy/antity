#include "str.h"
#include <stdio.h>
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


mb_StringView mb_str_split_from_right_till(mb_StringView *str, char c) {
  int64_t i = str->count;
  while(i-- != 0) {
    if(str->data[i] == c) break;
  }

  mb_StringView word = {0};
  // did not find
  if(i == -1) goto default_return;

  size_t next_index = i + 1;
  word.data = str->data + next_index;
  word.count = str->count - next_index;
  // 1 before. Remove the delim
  str->count = i != 0 ? i - 1 : 0;

default_return:
  return word;
}
