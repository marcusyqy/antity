#ifndef _MB_FILE_H_
#define _MB_FILE_H_

#include <stdio.h>
#include "str.h"

// @TODO: figure out naming schemes for the enums.
enum {
  MB_FILE_OPEN_MODE_WRITE = 1 << 1,
  MB_FILE_OPEN_MODE_TEXT = 1 << 2,

  mb_FileOpenMode_Write = 1 << 1,
  mb_FileOpenMode_Text = 1 << 2,
};
typedef uint8_t mb_FileOpenMode;

typedef FILE *mb_File;

mb_File       mb_open_file(const mb_StringView *str, mb_FileOpenMode open_mode);
void          mb_close_file(mb_File file);
mb_StringView mb_file_read_bytes(mb_Arena *arena, mb_File file);

typedef struct mb_FileNames {
  mb_StringView *names;
  size_t count;
} mb_FileNames;

mb_FileNames mb_glob(mb_Arena *arena, mb_StringView pattern);

#endif // _MB_FILE_H_

