#ifndef _LLVM_CONVERTER_H
#define _LLVM_CONVERTER_H

#include "common.h"
#include "parser.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

typedef struct {
    Parser *parser;

    LLVMContextRef context;
    LLVMBuilderRef builder;
    LLVMModuleRef module;
    LLVMTargetRef target;
    LLVMTargetMachineRef target_machine;
    char *target_triple;
} LLVM_Context;

bool llvm_init(LLVM_Context *ctx, Parser *parser);
void llvm_shutdown(LLVM_Context *ctx);
void llvm_convert(LLVM_Context *ctx);

// Use after llvm_convert
bool llvm_write_ir_file(LLVM_Context *ctx, const char *filepath);
bool llvm_emit_object(LLVM_Context *ctx, const char *filepath);

#endif /* _LLVM_CONVERTER_H */
