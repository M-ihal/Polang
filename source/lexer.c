#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

static inline wchar_t lexer_consume_char(Lexer *lexer) {
    wchar_t _char = str_view_consume_char(&lexer->file_view);
    if(_char == L'\n') {
        lexer->current_line += 1;
        // lexer->current_char = 0;
    }
    return _char;
}

static inline wchar_t lexer_peek_char(Lexer *lexer) {
    return str_view_peek(&lexer->file_view);
}

static inline wchar_t lexer_peek_char_next(Lexer *lexer, size_t offset) {
    return str_view_peek_next(&lexer->file_view, offset);
}

static inline void lexer_consume_whitespaces(Lexer *lexer) {
    while(iswspace(lexer_peek_char(lexer))) {
        lexer_consume_char(lexer);
    }
}

static inline bool starts_identifier(wchar_t _char) {
    return iswalpha(_char) || _char == L'_';
}

static inline bool continues_identifier(wchar_t _char) {
    return starts_identifier(_char) || iswdigit(_char);
}

static inline void lexer_push_token(Lexer *lexer, Token *token) {
    token->line = lexer->current_line; // @TODO:
    lexer->tokens[lexer->token_count++] = *token;
}

static inline void lexer_push_token_no_data(Lexer *lexer, Token_Kind kind) {
    Token token = { .kind = kind, .line = lexer->current_line };
    lexer_push_token(lexer, &token);
}

static void lexer_read_identifier(Lexer *lexer) {
    // Save starting point of identifier
    Str_View ident_view = lexer->file_view;

    size_t ident_length = 0;
    while(true) {
        if(!lexer->file_view.length) {
            // Read identifier to the end of input
            break;
        }

        const wchar_t _char = lexer_peek_char(lexer);

        if((!ident_length && starts_identifier(_char)) || (ident_length && continues_identifier(_char))) {
            ident_length += 1;
            lexer_consume_char(lexer);
        } else {
            // Not valid character for identifier
            break;
        }
    }

    if(ident_length) {
        // Set length of identifier in the saved view
        ident_view.length = ident_length;

        if(str_view_compare_to_string(ident_view, L"zwróć")) {
            lexer_push_token_no_data(lexer, TOKEN_KEYWORD_RETURN);
        } else if(str_view_compare_to_string(ident_view, L"nic")) {
            lexer_push_token_no_data(lexer, TOKEN_KEYWORD_VOID);
        } else if(str_view_compare_to_string(ident_view, L"całkowita64")) {
            lexer_push_token_no_data(lexer, TOKEN_KEYWORD_INT64);
        } else if(str_view_compare_to_string(ident_view, L"nieujemna64")) {
            lexer_push_token_no_data(lexer, TOKEN_KEYWORD_UINT64);
        } else if(str_view_compare_to_string(ident_view, L"rzeczywista64")) {
            lexer_push_token_no_data(lexer, TOKEN_KEYWORD_FLOAT64);
        } else {
            // Identifier
            Token token = { };
            token.kind = TOKEN_IDENTIFIER;
            token.value_string = ident_view;
            lexer_push_token(lexer, &token);
        }
    } else {
        // Did not read any valid identifier characters (Wrongly called procedure)
        assert(0);
    }
}

static void lexer_read_number(Lexer *lexer) {
    wchar_t number_buffer[128] = { };
    size_t  number_length = 0;

    bool dash_encountered = false;
    bool dot_encountered = false;

    // Get rid of starting minus
    if(lexer_peek_char(lexer) == L'-') {
        number_buffer[number_length++] = lexer_consume_char(lexer);
        dash_encountered = true;
    }

    // Insert 0 before single dot (Handle '.8' and '-.05' numbers)
    if(lexer_peek_char(lexer) == L'.') {
        number_buffer[number_length++] = L'0';
        number_buffer[number_length++] = lexer_consume_char(lexer);
        dot_encountered = true;
    }

    while(true) {
        if(!lexer->file_view.length) {
            // Read number to the end of input
            break;
        }

        const wchar_t _char = lexer_peek_char(lexer);

        if(!iswdigit(_char) && _char != L'.') {
            // Not valid number character
            break;
        }
    
        lexer_consume_char(lexer);

        if(_char == L'.') {
            if(dot_encountered) {
                // @TODO , @ERROR
                assert(0 && "Encountered second '.' while reading number...");
            } else {
                dot_encountered = true;
            }
        }

        number_buffer[number_length++] = _char;

        if(number_length >= (ARRAY_SIZE(number_buffer) - 1)) {
            // @TODO , @ERROR
            assert(0 && "Too long number; Number should never get that big!");
        }
    }

    // Make sure buffer is null-terminated, just in case
    number_buffer[number_length] = L'\0';

    if(number_length) {
        Token token = { };
        token.kind = TOKEN_NUMBER;

        if(dot_encountered) {
            token.value_float64 = wcstod(number_buffer, NULL);
            token.flags |= TOKEN_FLAG_NUMBER_FLOAT64;
        } else { // Integer
            if(!dash_encountered && number_length >= 10) {
                token.value_uint64 = wcstoll(number_buffer, NULL, 10);
                token.flags |= TOKEN_FLAG_NUMBER_UINT64;
            } else {
                token.value_int64 = wcstoll(number_buffer, NULL, 10);
                token.flags |= TOKEN_FLAG_NUMBER_INT64;
            }
        }

        lexer_push_token(lexer, &token);
    } else {
        // Did not read any valid number characters (Wrongly called procedure)
        assert(0);
    }
}

static void lexer_consume_until_next_line(Lexer *lexer) {
    while(true) {
        wchar_t _char = lexer_consume_char(lexer);

        if(_char == L'\n') {
            return;
        }

        if(_char == 0) {
            // End of file?
            return;
        }
    }
}

static void lexer_tokenize(Lexer *lexer) {
    lexer->current_line = 1;
    // lexer->current_char = 1;
    lexer->token_count  = 0;
    lexer->token_cursor = 0;
 
    lexer_consume_whitespaces(lexer);

    while(lexer->file_view.length) {
        lexer_consume_whitespaces(lexer);

        const wchar_t _char = lexer_peek_char(lexer);

        if(_char == (wchar_t)0) {
            break; // Reached end of the file
        } else if(starts_identifier(_char)) {
            lexer_read_identifier(lexer);
        } else if(iswdigit(_char)) {
            lexer_read_number(lexer);
        } else if(_char == L'-') {
            const wchar_t past_minus = lexer_peek_char_next(lexer, 1);
            if(iswdigit(past_minus)) {
                lexer_read_number(lexer);
            } else if(past_minus == L'>') {
                lexer_push_token_no_data(lexer, TOKEN_ARROW);
                lexer_consume_char(lexer);
                lexer_consume_char(lexer);
            } else {
                lexer_push_token_no_data(lexer, TOKEN_MINUS);
                lexer_consume_char(lexer);
            }
        } else if(_char == L'.') {
            const wchar_t past_dot = lexer_peek_char_next(lexer, 1);
            if(iswdigit(past_dot)) {
                lexer_read_number(lexer);
            } else {
                lexer_push_token_no_data(lexer, TOKEN_DOT);
                lexer_consume_char(lexer);
            }
        } else if(_char == L':') {
            const wchar_t past_colon = lexer_peek_char_next(lexer, 1);
            if(past_colon == L':') {
                lexer_push_token_no_data(lexer, TOKEN_COLON_DOUBLE);
                lexer_consume_char(lexer);
            } else {
                lexer_push_token_no_data(lexer, TOKEN_COLON);
            }
            lexer_consume_char(lexer);
        } else if(_char == L';') {
            lexer_push_token_no_data(lexer, TOKEN_SEMICOLON);
            lexer_consume_char(lexer);
        } else if(_char == L'(') {
            lexer_push_token_no_data(lexer, TOKEN_PAREN_OPEN);
            lexer_consume_char(lexer);
        } else if(_char == L')') {
            lexer_push_token_no_data(lexer, TOKEN_PAREN_CLOSE);
            lexer_consume_char(lexer);
        } else if(_char == L'{') {
            lexer_push_token_no_data(lexer, TOKEN_BRACE_OPEN);
            lexer_consume_char(lexer);
        } else if(_char == L'}') {
            lexer_push_token_no_data(lexer, TOKEN_BRACE_CLOSE);
            lexer_consume_char(lexer);
        } else if(_char == L',') {
            lexer_push_token_no_data(lexer, TOKEN_COMMA);
            lexer_consume_char(lexer);
        } else if(_char == L'+') {
            lexer_push_token_no_data(lexer, TOKEN_PLUS);
            lexer_consume_char(lexer);
        } else if(_char == L'*') {
            lexer_push_token_no_data(lexer, TOKEN_STAR);
            lexer_consume_char(lexer);
        } else if(_char == L'=') {
            lexer_push_token_no_data(lexer, TOKEN_EQUAL);
            lexer_consume_char(lexer);
        } else if(_char == L'/') {
            if(lexer_peek_char_next(lexer, 1) == L'/') {
                // Comment
                lexer_consume_until_next_line(lexer);
            } else {
                lexer_push_token_no_data(lexer, TOKEN_SLASH_FORWARD);
                lexer_consume_char(lexer);
            }
        } else if(_char == L'\\') {
            lexer_push_token_no_data(lexer, TOKEN_SLASH_BACKWARD);
            lexer_consume_char(lexer);
        } else {
            // No token recognized, skip the char?
            assert(0 && "Unrecognized character encountered in lexer_tokenize");
            lexer_consume_char(lexer);
        }
    }

    // Push EOF token at the end
    lexer_push_token_no_data(lexer, TOKEN_EOF);
}

static inline void lexer_set_file_data(Lexer *lexer, wchar_t *file_data, size_t file_length) {
    lexer->file_data = file_data;
    lexer->file_length = file_length;
    lexer->file_view = str_view(file_data, file_length);
}

bool lexer_init_from_file(Lexer *lexer, wchar_t *filepath) {
    ZERO_STRUCT(*lexer);

    wchar_t *file_data = NULL;
    size_t   file_length = 0;
    if(!file_read(filepath, &file_data, &file_length)) {
        fwprintf(stderr, L"Failed to read file while initializing lexer.\n");
        return false;
    }

    lexer_set_file_data(lexer, file_data, file_length);
    lexer_tokenize(lexer);

    return true;
}

void lexer_free(Lexer *lexer) {
    free(lexer->file_data);

    ZERO_STRUCT(*lexer);
}

void lexer_rewind(Lexer *lexer) {
    lexer->token_cursor = 0;
}

Token lexer_peek_token(Lexer *lexer, size_t offset) {
    const size_t cursor = lexer->token_cursor + offset;

    if(cursor >= lexer->token_count) {
        Token token_eof = { .kind = TOKEN_EOF };
        return token_eof;
    }

    return lexer->tokens[cursor];
}

Token lexer_next_token(Lexer *lexer) {
    if((lexer->token_cursor + 1) >= lexer->token_count) {
        Token token_eof = { .kind = TOKEN_EOF };
        return token_eof;
    }

    return lexer->tokens[lexer->token_cursor++];
}
