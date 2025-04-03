#include "parser.h"

#include <stdlib.h>

#define AST_NEW(parser, T) (T *)_parser_new_ast(parser, ast_kind(T), sizeof(T))

AST_Node *parse_expression(Parser *parser);

// @TODO:
void todo___print_the_line(Parser *parser, size_t line) {
    Str_View view = str_view(parser->lexer->file_data, parser->lexer->file_length);
 
    size_t lines_skipped = 1;
    while(true) {
        if(lines_skipped == line) {
            break;
        }
        size_t index = str_view_find_first(view, L'\n');
        str_view_consume(&view, index + 1);
        lines_skipped += 1;
    }

    str_view_consume_whitespaces(&view);
    size_t index = str_view_find_first(view, L'\n');
    if(index > 0) {
        view.length = index - 1;
    }
    
    wprintf(L"%.*ls", view.length, view.data);
}

void report_unexpected_token(Parser *parser, Token token, wchar_t *string) {
    wprintf(L"On line: %llu\nGot unexpected token %ls\n", token.line, token_kind_strings[token.kind]);
    if(token.kind == TOKEN_IDENTIFIER) {
        wprintf(L"Identifier = %.*ls\n", token.value_string.length, token.value_string.data);
    }
    todo___print_the_line(parser, token.line);
    wprintf(L"\n");
    if(string != NULL) {
        wprintf(string);
    }
    wprintf(L"\n");
    exit(-1);
}

void report_syntax_error(Parser *parser, Token token, Token_Kind token_expected) {
    wprintf(L"On line: %llu\nGot unexpected token %ls, expected %ls\n", token.line, token_kind_strings[token.kind], token_kind_strings[token_expected]);
    if(token.kind == TOKEN_IDENTIFIER) {
        wprintf(L"Identifier = %.*ls\n", token.value_string.length, token.value_string.data);
    }
    todo___print_the_line(parser, token.line);
    wprintf(L"\n");
    wprintf(L"\n");
    exit(-1);
}

static AST_Node *_parser_new_ast(Parser *parser, AST_Kind kind, size_t size_of_ast_struct) {
    AST_Node *new_ast = mem_arena_push(&parser->ast_mem_arena, size_of_ast_struct);
    if(new_ast == NULL) {
        assert(0 && "Failed to allocate memory for ast");
    }
    memset(new_ast, 0, size_of_ast_struct);
    new_ast->kind = kind;
    return new_ast;
}

bool parser_init(Parser *parser, Lexer *lexer) {
    ZERO_STRUCT(*parser);

    parser->ast_mem_arena = mem_arena_alloc(PARSER_AST_MEMORY_BYTES);

    if(parser->ast_mem_arena.pointer == NULL) {
        fwprintf(stderr, L"Failed to allocate memory for parser.\n");
        return false;
    }

    parser->lexer = lexer;
    return true;
}

void parser_free(Parser *parser) {
    mem_arena_free(&parser->ast_mem_arena);

    ZERO_STRUCT(*parser);
}

static inline Token expect_token(Parser *parser, Token_Kind expected_kind) {
    Token token = lexer_next_token(parser->lexer);
    if(token.kind != expected_kind) {
        report_syntax_error(parser, token, expected_kind);
    }
    return token;
}

AST_Type_Def *get_simple_data_type(Parser *parser, Token_Kind token_kind) {
    switch(token_kind) {
        default: return NULL;
        case TOKEN_KEYWORD_INT64:   return parser->ast_type_def_int64; 
        case TOKEN_KEYWORD_UINT64:  return parser->ast_type_def_uint64;
        case TOKEN_KEYWORD_FLOAT64: return parser->ast_type_def_float64;
    }
}

void root_add_node(AST_Root *ast_root, AST_Node *node) {
    assert(ast_root->nodes_count < AST_ROOT_NODES_MAX && "Exceeded root nodes limit @TODO");
    ast_root->nodes[ast_root->nodes_count++] = node;
}

void block_add_node(AST_Block *ast_block, AST_Node *node) {
    assert(ast_block->nodes_count < AST_BLOCK_NODES_MAX && "Exceeded block nodes limit @TODO");
    ast_block->nodes[ast_block->nodes_count++] = node;
}

AST_Parameter *parse_procedure_param(Parser *parser) {
    Token token_ident = lexer_peek_token(parser->lexer, 0);
    Token token_colon = lexer_peek_token(parser->lexer, 1);
    Token token_type  = lexer_peek_token(parser->lexer, 2);

    if(token_ident.kind != TOKEN_IDENTIFIER) {
        report_syntax_error(parser, token_ident, TOKEN_IDENTIFIER);
    }

    if(token_colon.kind != TOKEN_COLON) {
        report_syntax_error(parser, token_colon, TOKEN_COLON);
    }

    AST_Type_Def *ast_type = get_simple_data_type(parser, token_type.kind);
    if(ast_type == NULL) {
        report_unexpected_token(parser, token_type, L"Expected data type!");
    }

    lexer_next_token(parser->lexer);
    lexer_next_token(parser->lexer);
    lexer_next_token(parser->lexer);

    AST_Parameter *ast_param = AST_NEW(parser, AST_Parameter);
    ast_param->identifier = token_ident.value_string;
    ast_param->data_type = ast_type;
    return ast_param;
}

static void procedure_add_param(AST_Procedure *ast_proc, AST_Parameter *ast_param) {
    assert(ast_proc->params_count < AST_PROCEDURE_PARAMS_MAX && "Exceeded procedure parameter limit! @TODO");
    ast_proc->params[ast_proc->params_count++] = ast_param;
}

AST_Literal *make_ast_literal(Parser *parser, Token *token) {
    // Assumes token is a number
    assert(token->kind == TOKEN_NUMBER);

    AST_Literal *ast_literal = AST_NEW(parser, AST_Literal);

    if(token->flags & TOKEN_FLAG_NUMBER_INT64) {
        ast_literal->kind = LITERAL_INT64;
        ast_literal->value_int64 = token->value_int64;
    } else if(token->flags & TOKEN_FLAG_NUMBER_UINT64) {
        ast_literal->kind = LITERAL_UINT64;
        ast_literal->value_uint64 = token->value_uint64;
    } else if(token->flags & TOKEN_FLAG_NUMBER_FLOAT64) {
        ast_literal->kind = LITERAL_FLOAT64;
        ast_literal->value_float64 = token->value_float64;
    } else {
        // Gotta be one of the above
        assert(0);
    }

    return ast_literal;
}

void procedure_call_add_param(AST_Procedure_Call *ast_proc_call, AST_Node *expression) {
    assert(ast_proc_call->params_count < AST_PROCEDURE_PARAMS_MAX && "Exceeded procedure call params limit @TODO");
    ast_proc_call->params[ast_proc_call->params_count++] = expression;
}

AST_Procedure_Call *parse_procedure_call(Parser *parser) {
    Token token_signature = expect_token(parser, TOKEN_IDENTIFIER);
    expect_token(parser, TOKEN_PAREN_OPEN);

    AST_Procedure_Call *ast_proc_call = AST_NEW(parser, AST_Procedure_Call);
    ast_proc_call->procedure_signature = token_signature.value_string;
 
    // If next token is close paren, immediatelly fall off from following loop, could just wrap it in an if...
    bool expect_expression = lexer_peek_token(parser->lexer, 0).kind != TOKEN_PAREN_CLOSE;

    /* Parse parameters */
    while(true) {
        Token token = lexer_peek_token(parser->lexer, 0);

        if(token.kind == TOKEN_PAREN_CLOSE) {
            if(expect_expression) {
                report_unexpected_token(parser, token, L"Expected expression in a procedure call but got )");
            } else {
                lexer_next_token(parser->lexer);
                break;
            }
        } else if(token.kind == TOKEN_COMMA) {
            if(expect_expression) {
                report_unexpected_token(parser, token, L"Expected expression in a procedure call but got ,");
            } else {
                lexer_next_token(parser->lexer);
                expect_expression = true;
                continue;
            }
        } else {
            AST_Node *expression = parse_expression(parser);
            procedure_call_add_param(ast_proc_call, expression);
            expect_expression = false;
        }
    }

    return ast_proc_call;
}

AST_Node *parse_expression(Parser *parser) {
    AST_Node *expression = NULL;
 
    Token token = lexer_peek_token(parser->lexer, 0);
    switch(token.kind) {
        default: { report_unexpected_token(parser, token, L"Unexpected token in parse_expression"); } break;
        
        case TOKEN_NUMBER: {
            lexer_next_token(parser->lexer);
            
            AST_Literal *ast_literal = make_ast_literal(parser, &token);
            expression = (AST_Node *)ast_literal;
        } break;

        case TOKEN_IDENTIFIER: {
            Token token_past_ident = lexer_peek_token(parser->lexer, 1);
            if(token_past_ident.kind == TOKEN_PAREN_OPEN) {
                AST_Procedure_Call *ast_proc_call = parse_procedure_call(parser);
                expression = (AST_Node *)ast_proc_call;
            } else {
                lexer_next_token(parser->lexer);
                AST_Variable_Ref *ast_var_ref = AST_NEW(parser, AST_Variable_Ref);
                ast_var_ref->var_ident = token.value_string;
                expression = (AST_Node *)ast_var_ref;
            }
        } break;
    }

    Token token_next = lexer_peek_token(parser->lexer, 0);
    switch(token_next.kind) {
        default: {
            // Next token is not an operator so done with expression
        } break;

        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH_FORWARD: {
            lexer_next_token(parser->lexer);

            Binary_Operation operation;
            switch(token_next.kind) {
                default: assert(0);
                case TOKEN_PLUS:          operation = BINARY_OP_ADD; break;
                case TOKEN_MINUS:         operation = BINARY_OP_SUB; break;
                case TOKEN_STAR:          operation = BINARY_OP_MUL; break;
                case TOKEN_SLASH_FORWARD: operation = BINARY_OP_DIV; break;
            }

            AST_Binary *ast_binary = AST_NEW(parser, AST_Binary);
            ast_binary->operation = operation;
            ast_binary->expr_l = expression;
            ast_binary->expr_r = parse_expression(parser);
            expression = (AST_Node *)ast_binary;
        } break;
    }

    return expression;
}

AST_Block *parse_block(Parser *parser) {
    expect_token(parser, TOKEN_BRACE_OPEN);

    AST_Block *ast_block = AST_NEW(parser, AST_Block);

    while(true) {
        Token token = lexer_peek_token(parser->lexer, 0);
 
        if(token.kind == TOKEN_BRACE_CLOSE) {
            lexer_next_token(parser->lexer);
            break;
        }

        switch(token.kind) {
            case TOKEN_EOF: // Reached end of file while parsing block @TODO , @ERROR
            default: {
                report_unexpected_token(parser, token, L"Unexpected token in parse_block"); 
            } break;

            case TOKEN_KEYWORD_RETURN: {
                AST_Return *ast_return = AST_NEW(parser, AST_Return);
                lexer_next_token(parser->lexer);

                Token past_return = lexer_peek_token(parser->lexer, 0);
                if(past_return.kind == TOKEN_SEMICOLON) {
                    // Return from the procedure
                    ast_return->expression = NULL;
                } else {
                    ast_return->expression = parse_expression(parser);
                }

                expect_token(parser, TOKEN_SEMICOLON);

                block_add_node(ast_block, (AST_Node *)ast_return);
            } break;

            case TOKEN_IDENTIFIER: {
                lexer_next_token(parser->lexer);
                expect_token(parser, TOKEN_COLON);

                Token token_past_colon = lexer_peek_token(parser->lexer, 0);
                AST_Type_Def *ast_type_def = get_simple_data_type(parser, token_past_colon.kind);
                if(ast_type_def == NULL) {
                    report_unexpected_token(parser, token_past_colon, L"Expected data type for the identifier :");
                }

                lexer_next_token(parser->lexer);

                AST_Declaration *ast_decl = AST_NEW(parser, AST_Declaration);
                ast_decl->identifier = token.value_string;
                ast_decl->data_type = ast_type_def;

                if(lexer_peek_token(parser->lexer, 0).kind == TOKEN_EQUAL) {
                    lexer_next_token(parser->lexer);
                    ast_decl->expression = parse_expression(parser);
                    // if(ast_decl->expression == NULL) @TODO EXPECTED EXPRESSION
                } else {
                    ast_decl->expression = NULL;
                }

                expect_token(parser, TOKEN_SEMICOLON);

                block_add_node(ast_block, (AST_Node *)ast_decl);
            } break;
        }
    }

    return ast_block;
}

void parse_procedure(Parser *parser) {
    Token token_signature = expect_token(parser, TOKEN_IDENTIFIER);
    expect_token(parser, TOKEN_COLON_DOUBLE);
    expect_token(parser, TOKEN_PAREN_OPEN);

    AST_Procedure *ast_proc = AST_NEW(parser, AST_Procedure);
    ast_proc->signature = token_signature.value_string;

    // If immediatelly after there is ), do not expect a param, and fall out of the while loop
    bool expect_param = lexer_peek_token(parser->lexer, 0).kind != TOKEN_PAREN_CLOSE;

    /* Parse procedure parameters */
    while(true) {
        Token token = lexer_peek_token(parser->lexer, 0);

        if(token.kind == TOKEN_PAREN_CLOSE) {
            if(expect_param) {
                report_unexpected_token(parser, token, L"Expected a parameter in procedure parameter list but got )");
            } else {
                break;
            }
        } else if(token.kind == TOKEN_COMMA) {
            if(expect_param) {
                report_unexpected_token(parser, token, L"Expected a parameter in procedure parameter list but got ,");
            } else {
                lexer_next_token(parser->lexer);
                expect_param = true;
                continue;
            }
        } else {
            AST_Parameter *ast_param = parse_procedure_param(parser);
            assert(ast_param != NULL && "Failed to parse procedure param");
            procedure_add_param(ast_proc, ast_param);
            expect_param = false;
        }
    }

    // Expecting closing paren after parsing params
    expect_token(parser, TOKEN_PAREN_CLOSE);

    // Return type
    if(lexer_peek_token(parser->lexer, 0).kind == TOKEN_ARROW) {
        lexer_next_token(parser->lexer);
        Token token_type = lexer_next_token(parser->lexer);

        AST_Type_Def *ast_type = get_simple_data_type(parser, token_type.kind);

        if(ast_type == NULL) {
            if(token_type.kind == TOKEN_KEYWORD_VOID) {
                ast_type = parser->ast_type_def_void;
            } else {
                report_unexpected_token(parser, token_type, L"Expected data type!");
            }
        }

        ast_proc->return_type = ast_type;
    } else {
        ast_proc->return_type = parser->ast_type_def_void;
    }

    ast_proc->block = parse_block(parser);

    root_add_node(parser->ast_root, (AST_Node *)ast_proc);
}

void parser_parse(Parser *parser) {
    mem_arena_reset(&parser->ast_mem_arena);

    parser->ast_type_def_void = AST_NEW(parser, AST_Type_Def);
    parser->ast_type_def_void->kind = TYPE_VOID;
    parser->ast_type_def_void->signature = str_view_wcstr(L"void");

    parser->ast_type_def_int64 = AST_NEW(parser, AST_Type_Def);
    parser->ast_type_def_int64->kind = TYPE_INT64;
    parser->ast_type_def_int64->signature = str_view_wcstr(L"int64");

    parser->ast_type_def_uint64 = AST_NEW(parser, AST_Type_Def);
    parser->ast_type_def_uint64->kind = TYPE_UINT64;
    parser->ast_type_def_uint64->signature = str_view_wcstr(L"uint64");

    parser->ast_type_def_float64 = AST_NEW(parser, AST_Type_Def);
    parser->ast_type_def_float64->kind = TYPE_FLOAT64;
    parser->ast_type_def_float64->signature = str_view_wcstr(L"float64");

    parser->ast_root = AST_NEW(parser, AST_Root);

    while(true) {
        Token token = lexer_peek_token(parser->lexer, 0);

        if(token.kind == TOKEN_EOF) {
            // Reached end of token stream
            break;
        }

        if(token.kind == TOKEN_IDENTIFIER) {
            Token token_past_ident = lexer_peek_token(parser->lexer, 1);

            if(token_past_ident.kind == TOKEN_COLON_DOUBLE) {
                Token token_past_colons = lexer_peek_token(parser->lexer, 2);

                if(token_past_colons.kind == TOKEN_PAREN_OPEN) {
                    parse_procedure(parser);
                } 
            } else {
                lexer_next_token(parser->lexer);
            }
        } else {
            report_unexpected_token(parser, token, NULL);
            lexer_next_token(parser->lexer);
        }
    }
}
