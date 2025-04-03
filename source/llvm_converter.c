#include "llvm_converter.h"

#include <stdio.h>

// For now allocating all over the place and converting wide strings to multibyte string and stuff which is bad

#define DEFAULT_INVALID default: { assert(0 && "Invalid default case in a switch statement"); }

bool llvm_init(LLVM_Context *ctx, Parser *parser) {
    ZERO_STRUCT(*ctx);

    ctx->parser = parser;

    ctx->context = LLVMContextCreate();
    ctx->module  = LLVMModuleCreateWithNameInContext("module", ctx->context);
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);

    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    ctx->target_triple = LLVMGetDefaultTargetTriple();

    char *error_message = NULL;
    if(LLVMGetTargetFromTriple(ctx->target_triple, &ctx->target, &error_message) != 0) {
        fprintf(stderr, "LLVM ERROR: %s\n", error_message);
        LLVMDisposeMessage(error_message);

        return false; // @TODO Free resources
    }

    ctx->target_machine = LLVMCreateTargetMachine(ctx->target, ctx->target_triple, LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelNone, LLVMRelocDefault, LLVMCodeModelDefault);

    return true;
}

void llvm_shutdown(LLVM_Context *ctx) {
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    LLVMContextDispose(ctx->context);
    LLVMDisposeMessage(ctx->target_triple);

    ZERO_STRUCT(*ctx);
}

// Only used for simple types @TODO Change when structs
LLVMTypeRef get_llvm_simple_type(LLVM_Context *ctx, Type_Kind kind) {
    switch(kind) { DEFAULT_INVALID;
        case TYPE_VOID: {
            return LLVMVoidTypeInContext(ctx->context);
        };
        case TYPE_INT64: {
            return LLVMInt64TypeInContext(ctx->context);
        };
        case TYPE_UINT64: {
            return LLVMInt64TypeInContext(ctx->context);
        };
        case TYPE_FLOAT64: {
            return LLVMDoubleTypeInContext(ctx->context);
        };
    }
}

LLVMTypeRef get_llvm_literal_simple_type(LLVM_Context *ctx, Literal_Kind kind) {
    switch(kind) { DEFAULT_INVALID;
        case LITERAL_INT64: {
            return LLVMInt64TypeInContext(ctx->context);
        };
        case LITERAL_UINT64: {
            return LLVMInt64TypeInContext(ctx->context);
        };
        case LITERAL_FLOAT64: {
            return LLVMDoubleTypeInContext(ctx->context);
        };
    }
}

typedef struct {
    const char *ident_string; // Allocated for now @TODO
    LLVMValueRef value_ref;
    Type_Kind type;
} Scope_Symbol;

#define SCOPE_SYMBOLS_MAX 1024

typedef struct {
    LLVMBasicBlockRef basic_block;

    // For variable lookup
    Scope_Symbol symbols[SCOPE_SYMBOLS_MAX];
    size_t symbols_count;
} LLVM_Scope;

void llvm_scope_init(LLVM_Scope *scope) {
    scope->symbols_count = 0;
}

void llvm_scope_add_symbol(LLVM_Scope *scope, LLVMValueRef ref, const char *ident, Type_Kind type) {
    assert(scope->symbols_count < SCOPE_SYMBOLS_MAX && "Exceeded LLVM_Scope symbols limit @TODO");
    Scope_Symbol symbol = (Scope_Symbol) { .ident_string = ident, .value_ref = ref, .type = type };
    scope->symbols[scope->symbols_count++] = symbol;
}

// @TODO: Hash(?)
Scope_Symbol llvm_scope_lookup_symbol(LLVM_Scope *scope, const char *ident) {
    for(size_t index = 0; index < scope->symbols_count; ++index) {
        if(strcmp(ident, scope->symbols[index].ident_string) == 0) {
            return scope->symbols[index];
        }
    }
    return (Scope_Symbol) { };
}

LLVMValueRef make_llvm_expression(LLVM_Context *ctx, AST_Node *expr, LLVM_Scope *scope) {
     switch(expr->kind) { DEFAULT_INVALID;

        case ast_kind(AST_Literal): {
            AST_Literal *ast_literal = (AST_Literal *)expr;
            switch(ast_literal->kind) { DEFAULT_INVALID;
                case LITERAL_INT64: {
                    return LLVMConstInt(get_llvm_literal_simple_type(ctx, ast_literal->kind), (uint64_t)ast_literal->value_int64, 1);
                };
                case LITERAL_UINT64: {
                    return LLVMConstInt(get_llvm_literal_simple_type(ctx, ast_literal->kind), ast_literal->value_uint64, 0);
                };
                case LITERAL_FLOAT64: {
                    return LLVMConstReal(get_llvm_literal_simple_type(ctx, ast_literal->kind), ast_literal->value_float64);
                };
            }
        } break;

        case ast_kind(AST_Binary): {
            AST_Binary *ast_binary = (AST_Binary *)expr;

            LLVMValueRef expr_l = make_llvm_expression(ctx, ast_binary->expr_l, scope);
            LLVMValueRef expr_r = make_llvm_expression(ctx, ast_binary->expr_r, scope);

            switch(ast_binary->operation) { DEFAULT_INVALID;
                case BINARY_OP_ADD: {
                    return LLVMBuildAdd(ctx->builder, expr_l, expr_r, "a");
                };
                case BINARY_OP_SUB: {
                    return LLVMBuildSub(ctx->builder, expr_l, expr_r, "s");
                };
                case BINARY_OP_MUL: {
                    return LLVMBuildMul(ctx->builder, expr_l, expr_r, "m");
                };
                case BINARY_OP_DIV: {
                    return LLVMBuildSDiv(ctx->builder, expr_l, expr_r, "d");
                };
            }
        } break;

        case ast_kind(AST_Variable_Ref): {
            AST_Variable_Ref *ast_var_ref = (AST_Variable_Ref *)expr;
            
            char *var_ident = convert_to_mbs_alloc(ast_var_ref->var_ident.data, ast_var_ref->var_ident.length, NULL);
            assert(var_ident);
            Scope_Symbol symbol = llvm_scope_lookup_symbol(scope, var_ident);
            free(var_ident);

            LLVMValueRef loaded = LLVMBuildLoad2(ctx->builder, get_llvm_simple_type(ctx, symbol.type), symbol.value_ref, "");
            return loaded;
        } break;
    }
}

void emit_return(LLVM_Context *ctx, AST_Return *ast_ret, LLVM_Scope *scope) {
    if(ast_ret->expression == NULL) {
        LLVMBuildRetVoid(ctx->builder);
        return;
    }
    
    LLVMValueRef expr = make_llvm_expression(ctx, ast_ret->expression, scope);
    LLVMBuildRet(ctx->builder, expr);
}

void emit_procedure(LLVM_Context *ctx, AST_Procedure *ast_proc) {

    LLVMTypeRef return_type = get_llvm_simple_type(ctx, ast_proc->return_type->kind);
    
    LLVMTypeRef param_types[AST_PROCEDURE_PARAMS_MAX];
    
    for(size_t index = 0; index < ast_proc->params_count; ++index) {
        AST_Parameter *ast_param = ast_proc->params[index];
        param_types[index] = get_llvm_simple_type(ctx, ast_param->data_type->kind);
    }

    LLVMTypeRef proc_type = LLVMFunctionType(return_type, param_types, ast_proc->params_count, 0);

    // @TODO: Use temporary buffer 
    char *proc_signature = convert_to_mbs_alloc(ast_proc->signature.data, ast_proc->signature.length, NULL);

    LLVMValueRef proc = LLVMAddFunction(ctx->module, proc_signature, proc_type);

    free(proc_signature);

    LLVMBasicBlockRef block = LLVMAppendBasicBlock(proc, "block");

    LLVMPositionBuilderAtEnd(ctx->builder, block);

    LLVM_Scope scope;
    llvm_scope_init(&scope);

    AST_Block *ast_block = ast_proc->block;

    for(size_t index = 0; index < ast_block->nodes_count; ++index) {
        AST_Node *node = ast_block->nodes[index];
 
        if(node->kind == ast_kind(AST_Return)) {
            emit_return(ctx, (AST_Return *)node, &scope);
        }

        if(node->kind == ast_kind(AST_Declaration)) {
            AST_Declaration *ast_decl = (AST_Declaration *)node;
            
            // @TODO Do not allocate
            char *var_ident = convert_to_mbs_alloc(ast_decl->identifier.data, ast_decl->identifier.length, NULL);

            LLVMValueRef var_decl = LLVMBuildAlloca(ctx->builder, get_llvm_simple_type(ctx, ast_decl->data_type->kind), var_ident);

            if(ast_decl->expression != NULL) {
                LLVMValueRef expr = make_llvm_expression(ctx, ast_decl->expression, &scope);
                LLVMBuildStore(ctx->builder, expr, var_decl);
            }

            llvm_scope_add_symbol(&scope, var_decl, var_ident, ast_decl->data_type->kind);
            // free(var_ident);
        }
    }
}

void llvm_convert(LLVM_Context *ctx) {
    AST_Root *ast_root = ctx->parser->ast_root;

    for(size_t index = 0; index < ast_root->nodes_count; ++index) {
        AST_Node *node = ast_root->nodes[index];
        if(node->kind == ast_kind(AST_Procedure)) {
            emit_procedure(ctx, (AST_Procedure *)node);
        }
    }

    LLVMVerifyModule(ctx->module, LLVMAbortProcessAction, NULL);
}

bool llvm_write_ir_file(LLVM_Context *ctx, const char *filepath) {
    char *error_message = NULL;
    bool success = LLVMPrintModuleToFile(ctx->module, filepath, &error_message) == 0;
    if(!success) {
        fprintf(stderr, "LLVM PRINT TO FILE ERROR: %s\n", error_message);
        LLVMDisposeMessage(error_message);
        return false;
    }
    return true;
}

bool llvm_emit_object(LLVM_Context *ctx, const char *filepath) {
    char *error_message = NULL;
    bool success = LLVMTargetMachineEmitToFile(ctx->target_machine, ctx->module, filepath, LLVMObjectFile, &error_message) == 0;
    if(!success) {
        fprintf(stderr, "LLVM EMIT ERROR: %s\n", error_message);
        LLVMDisposeMessage(error_message);
        return false;
    }
    return true;
}
