#ifndef _COMMON_H
#define _COMMON_H

#include <wchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#define KB(B) ((B) * 1024)
#define MB(B) (KB(B) * 1024)
#define GB(B) (MB(B) * 1024)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define ZERO_STRUCT(s) memset(&(s), 0, sizeof(s))
#define ZERO_ARRAY(arr) memset(arr, 0, sizeof(arr))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define INVALID_CODE_PATH assert(0 && "Invalid code path!")
#define NOT_IMPLEMENTED assert(0 && "Not implemented path!")

// @NOTE : Converts char buffer to wcs; Includes null-term char; Returns NULL on failure; out_converted counts null-term, gets set on success
wchar_t *convert_to_wcs_alloc(const char *src, size_t src_length, size_t *out_converted);

// @NOTE : Converts char buffer into given dest array which must accomodate for null-term char; truncates if not enough space; Returns 0 on failure
size_t convert_to_wcs(const char *src, size_t src_length, wchar_t *dest, size_t dest_size);

char *convert_to_mbs_alloc(const wchar_t *src, size_t src_length, size_t *out_converted);

#endif /* _COMMON_H */
