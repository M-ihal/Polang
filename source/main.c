#include "common.h"
#include "file_io.h"
#include "lexer.h"
#include "parser.h"
#include "llvm_converter.h"

#include <stdio.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define PRINT_AST_TREE_DEPTH_MAX 128

void print_ast(AST_Node *node, int32_t depth, bool is_last, bool depth_continues[PRINT_AST_TREE_DEPTH_MAX]) {
    if(depth > -1) {
        for(int32_t index = 0; index < depth; ++index) {
            if(depth_continues[index]) {
                wprintf(L"│   ");
            } else {
                wprintf(L"    ");
            }
        }

        if(is_last) {
            wprintf(L"└── ");
        } else {
            wprintf(L"├── ");
        }

        depth_continues[depth] = !is_last;
    }

    switch(node->kind) {
        default: {
            assert(0 && "Unhandled AST_Kind in print_ast!");
        } break;

        case ast_kind(AST_Root): {
            AST_Root *ast_root = (AST_Root *)node;
            wprintf(L"Root\n");

            for(size_t index = 0; index < ast_root->nodes_count; ++index) {
                AST_Node *sub_node = ast_root->nodes[index];
                print_ast(sub_node, depth + 1, (index + 1) == ast_root->nodes_count, depth_continues);
            }
        } break;

        case ast_kind(AST_Type_Def): {
            AST_Type_Def *ast_type_def = (AST_Type_Def *)node;
            wprintf(L"Type Def : %.*ls, size : %lluB\n", ast_type_def->signature.length, ast_type_def->signature.data, get_size_of_type(ast_type_def->kind));
        } break;

        case ast_kind(AST_Parameter): {
            AST_Parameter *ast_param = (AST_Parameter *)node;
            wprintf(L"Parameter : %.*ls\n", ast_param->identifier.length, ast_param->identifier.data);
            print_ast((AST_Node *)ast_param->data_type, depth + 1, true, depth_continues);
        } break;

        case ast_kind(AST_Declaration): {
            AST_Declaration *ast_decl = (AST_Declaration *)node;
            wprintf(L"Declaration : %.*ls\n", ast_decl->identifier.length, ast_decl->identifier.data);
            print_ast((AST_Node *)ast_decl->data_type, depth + 1, ast_decl->expression == NULL, depth_continues);
            if(ast_decl->expression != NULL) {
                print_ast(ast_decl->expression, depth + 1, true, depth_continues);
            }
        } break;

        case ast_kind(AST_Block): {
            AST_Block *ast_block = (AST_Block *)node;
            wprintf(L"Block\n");

            for(size_t index = 0; index < ast_block->nodes_count; ++index) {
                AST_Node *sub_node = ast_block->nodes[index];
                print_ast(sub_node, depth + 1, (index + 1) == ast_block->nodes_count, depth_continues);
            }
        } break;

        case ast_kind(AST_Procedure): {
            AST_Procedure *ast_proc = (AST_Procedure *)node;
            wprintf(L"Procedure : %.*ls\n", ast_proc->signature.length, ast_proc->signature.data);

            print_ast((AST_Node *)ast_proc->return_type, depth + 1, ast_proc->params_count == 0 && ast_proc->block == NULL, depth_continues);

            for(size_t index = 0; index < ast_proc->params_count; ++index) {
                AST_Parameter *ast_param = ast_proc->params[index];
                print_ast((AST_Node *)ast_param, depth + 1, ast_proc->block == NULL && (index + 1) == ast_proc->params_count, depth_continues);
            }

            if(ast_proc->block != NULL) {
                print_ast((AST_Node *)ast_proc->block, depth + 1, true, depth_continues);
            }
        } break;

        case ast_kind(AST_Return): {
            AST_Return *ast_return = (AST_Return *)node;
            wprintf(L"Return\n");

            if(ast_return->expression != NULL) {
                print_ast(ast_return->expression, depth + 1, true, depth_continues);
            }
        } break;

        case ast_kind(AST_Literal): {
            AST_Literal *ast_literal = (AST_Literal *)node;
            wprintf(L"Literal : ");
            
            switch(ast_literal->kind) {
                default: assert(0 && "Unhandled literal type in print_ast"); break;
                case LITERAL_INT64:   { wprintf(L"%lld [int64]\n", ast_literal->value_int64); } break;
                case LITERAL_UINT64:  { wprintf(L"%llu [uint64]\n", ast_literal->value_uint64); } break;
                case LITERAL_FLOAT64: { wprintf(L"%f [float64]\n", ast_literal->value_float64); } break;
            }
        } break;

        case ast_kind(AST_Binary): {
            AST_Binary *ast_binary = (AST_Binary *)node;
            wprintf(L"Binary : %ls\n", binary_operation_string(ast_binary->operation));
            print_ast(ast_binary->expr_l, depth + 1, false, depth_continues);
            print_ast(ast_binary->expr_r, depth + 1, true, depth_continues);
        } break;

        case ast_kind(AST_Variable_Ref): {
            AST_Variable_Ref *ast_var_ref = (AST_Variable_Ref *)node;
            wprintf(L"Variable reference : %.*ls\n", ast_var_ref->var_ident.length, ast_var_ref->var_ident.data);
        } break;

        case ast_kind(AST_Procedure_Call): {
            AST_Procedure_Call *ast_proc_call = (AST_Procedure_Call *)node;
            wprintf(L"Procedure call : %.*ls\n", ast_proc_call->procedure_signature.length, ast_proc_call->procedure_signature.data);

            for(size_t index = 0; index < ast_proc_call->params_count; ++index) {
                AST_Node *expression = ast_proc_call->params[index];
                assert(is_expression(expression) && "Procedure call's param is not an expression !!!");
                print_ast(expression, depth + 1, (index + 1) == ast_proc_call->params_count, depth_continues);
            }
        } break;
    }
}

static void print_ast_tree(Parser *parser) {
    wprintf(L"\nGenerated AST Tree\n");
    wprintf(L"-----------------\n");
    bool depth_continues[PRINT_AST_TREE_DEPTH_MAX] = { };
    print_ast((AST_Node *)parser->ast_root, -1, true, depth_continues);
    wprintf(L"-----------------\n\n");
}

static void print_lexer_tokens(Lexer *lexer) {
    while(true) {
        Token token = lexer_next_token(lexer);

        wprintf(L"%s", token_kind_strings[token.kind]);

        switch(token.kind) {
            default: break;

            case TOKEN_IDENTIFIER: {
                wprintf(L" : %.*ls", token.value_string.length, token.value_string.data);
            } break;

            case TOKEN_NUMBER: {
                if(token.flags & TOKEN_FLAG_NUMBER_INT64) {
                    wprintf(L" : %lld", token.value_int64);
                } else if(token.flags & TOKEN_FLAG_NUMBER_UINT64) {
                    wprintf(L" : %lld", token.value_uint64);
                } else if(token.flags & TOKEN_FLAG_NUMBER_FLOAT64) {
                    wprintf(L" : %f", token.value_float64);
                } else {
                    wprintf(L" : Unknown number value!!!");
                }
            } break;
        }
            
        wprintf(L"\n");

        if(token.kind == TOKEN_EOF) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "en_US.utf-8");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    fwprintf(stdout, L"\nStart...\n");

    wchar_t source_file_path[64] = { };

    if(argc >= 2) { // Source file specified as command line argument
        size_t converted = convert_to_wcs(argv[1], strlen(argv[1]), source_file_path, ARRAY_SIZE(source_file_path));
    } else { // Use test source file path
        wcscpy_s(source_file_path, ARRAY_SIZE(source_file_path), L"../source/główny.polang");
    }

    assert(wcslen(source_file_path) > 0 && "Length of source file path is 0");
    fwprintf(stdout, L"Source file: \"%ls\"\n", source_file_path);

    Lexer lexer;
    if(!lexer_init_from_file(&lexer, source_file_path)) {
        return -1;
    }

    fwprintf(stdout, L"Lexed tokens: %llu\n", lexer.token_count);

    Parser parser;
    if(!parser_init(&parser, &lexer)) {
        lexer_free(&lexer);
        return -1;
    }

    lexer_rewind(&lexer);
    parser_parse(&parser);
 
    wprintf(L"Parsed without error\n");
    wprintf(L"AST memory usage: %llub of %llub (%f%%)\n", parser.ast_mem_arena.cursor, parser.ast_mem_arena.bytes, (double)parser.ast_mem_arena.cursor / (double)parser.ast_mem_arena.bytes);

    print_ast_tree(&parser);

    wprintf(L"LLVM converter init\n");

    LLVM_Context llvm_ctx;
    llvm_init(&llvm_ctx, &parser);

    llvm_convert(&llvm_ctx);

    const char *ir_file_path = "program_IR.txt";
    if(llvm_write_ir_file(&llvm_ctx, ir_file_path)) {
        fprintf(stdout, "IR file written to \"%s\"\n", ir_file_path);
    } else {
        fprintf(stderr, "Failed to write IR file.\n");
    }

    fwprintf(stdout, L"Freeing resources\n");

    lexer_free(&lexer);
    parser_free(&parser);
    llvm_shutdown(&llvm_ctx);

    fwprintf(stdout, L"\nExited successfully.\n");
    return 0;
}
