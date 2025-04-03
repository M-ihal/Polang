#ifndef _STRING_VIEW_H
#define _STRING_VIEW_H

#include "common.h"

/* Returned in functions that return size_t when not success */
#define STR_VIEW_FAIL ((size_t)-1)

/* string pointer must stay valid while using the Str_View */
typedef struct {
    const wchar_t *data;
    size_t length;
} Str_View;

/* Initializes Str_View struct */
Str_View str_view(const wchar_t *data, size_t length);

/* Initializes Str_View from null-terminated wstring */
Str_View str_view_wcstr(const wchar_t *string);

/* Returns true if both string are fully equal */
bool str_view_compare(Str_View view_a, Str_View view_b);
bool str_view_compare_to_string(Str_View view, const wchar_t *string);

/* Advances the string pointer; Returns number of chars consumed */
size_t  str_view_consume(Str_View *view, size_t num);
wchar_t str_view_consume_char(Str_View *view);
void    str_view_consume_whitespaces(Str_View *view);

/* Peeks char or STR_VIEW_FAIL if no more chars */
wchar_t str_view_peek(Str_View *view);
wchar_t str_view_peek_next(Str_View *view, size_t offset);

size_t str_view_find_first(Str_View view, wchar_t _char);
size_t str_view_find_first_of(Str_View view, wchar_t const *chars, size_t count);
size_t str_view_find_first_not(Str_View view, wchar_t _char);
size_t str_view_find_first_not_of(Str_View view, wchar_t const *chars, size_t count);

#endif /* _STRING_VIEW_H */
