#include "string_view.h"

Str_View str_view(const wchar_t *data, size_t length) {
    return (Str_View) {
        .data = data,
        .length = length
    };
}

Str_View str_view_wcstr(const wchar_t *string) {
    return (Str_View) {
        .data = string,
        .length = wcslen(string)
    };
}

bool str_view_compare(Str_View view_a, Str_View view_b) {
    if(view_a.length != view_b.length) {
        return false;
    }
    return memcmp(view_a.data, view_b.data, view_a.length * sizeof(wchar_t)) == 0;
}

bool str_view_compare_to_string(Str_View view, const wchar_t *string) {
    return str_view_compare(view, str_view_wcstr(string));
}

size_t str_view_consume(Str_View *view, size_t num) {
    const size_t to_consume = MIN(num, view->length);
    view->data += to_consume;
    view->length -= to_consume;
    return to_consume;
}

wchar_t str_view_consume_char(Str_View *view) {
    const wchar_t _char = str_view_peek(view);
    str_view_consume(view, 1);
    return _char;
}

void str_view_consume_whitespaces(Str_View *view) {
    while(view->length && iswspace(str_view_peek(view))) {
        str_view_consume_char(view);
    }
}

wchar_t str_view_peek(Str_View *view) {
    return str_view_peek_next(view, 0);
}

wchar_t str_view_peek_next(Str_View *view, size_t offset) {
    if(offset >= view->length) {
        return (wchar_t)0;
    }
    return view->data[offset];
}

static inline bool is_char_any_of(wchar_t _char, wchar_t const *chars, size_t count) {
    for(size_t index = 0; index < count; ++index) {
        if(chars[index] == _char) {
            return true;
        }
    }
    return false;
}

size_t str_view_find_first(Str_View view, wchar_t _char) {
    size_t index = 0;
    while(view.length && str_view_peek(&view) != _char) {
        str_view_consume_char(&view);
        index += 1;
    }
    return view.length == 0 ? STR_VIEW_FAIL : index;
}

size_t str_view_find_first_of(Str_View view, wchar_t const *chars, size_t count) {
    size_t index = 0;
    while(view.length && !is_char_any_of(str_view_peek(&view), chars, count)) {
        str_view_consume_char(&view);
        index += 1;
    }
    return view.length == 0 ? STR_VIEW_FAIL : index;
}

size_t str_view_find_first_not(Str_View view, wchar_t _char) {
    size_t index = 0;
    while(view.length && str_view_peek(&view) == _char) {
        str_view_consume_char(&view);
        index += 1;
    }
    return view.length == 0 ? STR_VIEW_FAIL : index;
}

size_t str_view_find_first_not_of(Str_View view, wchar_t const *chars, size_t count){
    size_t index = 0;
    while(view.length && is_char_any_of(str_view_peek(&view), chars, count)) {
        str_view_consume_char(&view);
        index += 1;
    }
    return view.length == 0 ? STR_VIEW_FAIL : index;
}
