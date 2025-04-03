#ifndef _LEXER_H
#define _LEXER_H

#include "common.h"
#include "file_io.h"
#include "string_view.h"

typedef enum : uint8_t {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_ARROW,
    TOKEN_COMMA,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_EQUAL,
    TOKEN_SLASH_FORWARD,
    TOKEN_SLASH_BACKWARD,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COLON_DOUBLE,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_DOT,

    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_INT64,
    TOKEN_KEYWORD_UINT64,
    TOKEN_KEYWORD_FLOAT64,
    TOKEN_KEYWORD_VOID,

    TOKEN__COUNT,
    TOKEN__INVALID
} Token_Kind;

static const wchar_t *token_kind_strings[TOKEN__COUNT] = {
    L"EOF",
    L"Identifier",
    L"Number",
    L"Arrow",
    L"Comma",
    L"Plus",
    L"Minus",
    L"Star",
    L"Equal",
    L"Forward slash",
    L"Backward slash",
    L"Semicolon",
    L"Colon",
    L"Double colon",
    L"Open paren",
    L"Close paren",
    L"Open brace",
    L"Close brace",
    L"Open bracket",
    L"Close bracket",
    L"Dot",

    L"Keyword return",
    L"Keyword int64",
    L"Keyword uint64",
    L"Keyword float64",
    L"Keyword void",
};

typedef enum : uint16_t {
    TOKEN_FLAG_NUMBER_INT64   = 0x1,
    TOKEN_FLAG_NUMBER_UINT64  = 0x2,
    TOKEN_FLAG_NUMBER_FLOAT64 = 0x4,

    TOKEN_FLAG_NUMBER_ANY = TOKEN_FLAG_NUMBER_INT64 | TOKEN_FLAG_NUMBER_UINT64 | TOKEN_FLAG_NUMBER_FLOAT64
} Token_Flags;

typedef struct {
    Token_Kind  kind;
    Token_Flags flags;

    // Where in file source
    size_t line;

    /* Values filled depending on Token_Kind */
    union {
        Str_View value_string; // View into the input string, stays valid with the lexer
        int64_t  value_int64;
        uint64_t value_uint64;
        double   value_float64;
    };
} Token;

typedef struct {
    // Lexer input
    wchar_t *file_data;
    size_t   file_length;
    Str_View file_view;

    // Used when tokenizing
    size_t current_line;
    // size_t current_char;

    // Generated tokens
    Token  tokens[4096]; // @TODO: Fixed size
    size_t token_count;
    size_t token_cursor;
} Lexer;

bool  lexer_init_from_file(Lexer *lexer, wchar_t *filepath);
void  lexer_free(Lexer *lexer);
void  lexer_rewind(Lexer *lexer);
Token lexer_peek_token(Lexer *lexer, size_t offset);
Token lexer_next_token(Lexer *lexer);

#endif /* _LEXER_H */
