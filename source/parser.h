#ifndef _PARSER_H
#define _PARSER_H

#include "common.h"
#include "lexer.h"
#include "memory_arena.h"
#include "ast_defs.h"

#define PARSER_AST_MEMORY_BYTES MB(16)

inline bool is_expression(AST_Node *node) {
    switch(node->kind) {
        default: {
            return false;
        }

        case ast_kind(AST_Literal):
        case ast_kind(AST_Binary):
        case ast_kind(AST_Variable_Ref):
        case ast_kind(AST_Procedure_Call): {
            return true;
        }
    }
}

typedef struct {
    // Memory for creating AST nodes
    Memory_Arena ast_mem_arena;

    // Input lexer
    Lexer *lexer;

    // Root of the AST tree
    AST_Root *ast_root;

    // Basic data types
    AST_Type_Def *ast_type_def_void;
    AST_Type_Def *ast_type_def_int64;
    AST_Type_Def *ast_type_def_uint64;
    AST_Type_Def *ast_type_def_float64;
} Parser;

bool parser_init(Parser *parser, Lexer *lexer);
void parser_free(Parser *parser);
void parser_parse(Parser *parser);

#endif /* _PARSER_H */
