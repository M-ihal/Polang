#ifndef _FILE_IO_H
#define _FILE_IO_H

#include "common.h"

/* out_chars -> @allocated; Includes null-terminator */
/* out_length -> Number of characters not including null-terminator */
bool file_read(const wchar_t *filepath, wchar_t **out_chars, size_t *out_length);

#endif /* _FILE_IO_H */
