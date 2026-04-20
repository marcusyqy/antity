#ifndef _MB_BASE_STR_
#define _MB_BASE_STR_

#include "arena.h"

typedef struct {
  char   *data;
  size_t count;
} mb_StringView;

mb_StringView  mb_str_from_cstr(const char *str);
char          *mb_str_to_cstr(mb_Arena *arena, mb_StringView str);
void           mb_str_to_cstr_inplace(char *cstr, mb_StringView str);

mb_StringView mb_str_split_from_right_till(mb_StringView *str, char c);

// printf stuff
#define MB_SV_FMT "%.*s"
#define MB_SV_VAR(s) ((int)(s).count), (s).data

#endif // _MB_BASE_STR_
