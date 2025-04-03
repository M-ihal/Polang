#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

wchar_t *convert_to_wcs_alloc(const char *src, size_t src_length, size_t *out_converted) {
#ifdef _WIN32
    int32_t wide_length = MultiByteToWideChar(CP_UTF8, 0, src, src_length, NULL, 0);
    if(!wide_length) {
        return NULL;
    }

    wchar_t *wide_buffer = (wchar_t *)malloc(wide_length * sizeof(wchar_t) + 1);
    if(wide_buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory in convert_to_wcs_alloc.\n");
        return NULL;
    }

    int32_t chars_written = MultiByteToWideChar(CP_UTF8, 0, src, src_length, wide_buffer, wide_length);
    if(!chars_written) {
        free(wide_buffer);
        return NULL;
    }
 
    wide_buffer[wide_length] = '\0';

    if(out_converted != NULL) {
       *out_converted = (size_t)chars_written;
    }

    return wide_buffer;
#else
    NOT_IMPLEMENTED;
#endif
}

// @TODO
size_t convert_to_wcs(const char *src, size_t src_length, wchar_t *dest, size_t dest_size) {
    size_t req_size = 0;
    if(dest_size == 0 || !!mbstowcs_s(&req_size, NULL, 0, src, src_length) || req_size == 0) {
        return 0;
    }

    size_t to_convert = MIN(req_size, dest_size);
    size_t chars_converted = 0;

    if(!!mbstowcs_s(&chars_converted, dest, dest_size, src, to_convert - 1)) {
        return 0;
    }

    return chars_converted;
}

char *convert_to_mbs_alloc(const wchar_t *src, size_t src_length, size_t *out_converted) {
#ifdef _WIN32
    int32_t mbs_length = WideCharToMultiByte(CP_UTF8, 0, src, src_length, NULL, 0, NULL, NULL);
    if(!mbs_length) {
        return NULL;
    }

    char *mbs_buffer = (char *)malloc(mbs_length * sizeof(char) + 1);
    if(mbs_buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory in convert_to_mbs_alloc.\n");
        return NULL;
    }

    int32_t bytes_written = WideCharToMultiByte(CP_UTF8, 0, src, src_length, mbs_buffer, mbs_length, NULL, NULL);
    if(!bytes_written) {
        free(mbs_buffer);
        return NULL;
    }
 
    mbs_buffer[mbs_length] = '\0';

    if(out_converted != NULL) {
        *out_converted = (size_t)bytes_written;
    }

    return mbs_buffer;
#else
    NOT_IMPLEMENTED;
#endif
}
