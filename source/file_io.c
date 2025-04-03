#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>

bool file_read(const wchar_t *filepath, wchar_t **out_chars, size_t *out_length) {
    FILE *file = NULL;
    if(_wfopen_s(&file, filepath, L"rb") != 0) {
        return false;
    }
 
    // Get file size
    fseek(file, 0, SEEK_END);
    long bytes = ftell(file);
    rewind(file);

    // Allocate memory and read entire file into the buffer
    char *buffer = malloc(bytes + 1);
    if(buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory in file_read.\n");
        fclose(file);
        return false;
    }

    fread(buffer, 1, bytes, file);
    buffer[bytes] = '\0';
    fclose(file);

    // Convert read file contents into wcs string
    size_t wide_length = 0;
    wchar_t *wide_buffer = convert_to_wcs_alloc(buffer, bytes, &wide_length);

    free(buffer);
    
    if(wide_buffer == NULL) {
        // Failed to convert to wcs buffer
        return false;
    }

    *out_chars = wide_buffer;
    *out_length = wide_length - 1;

    return true;
}
