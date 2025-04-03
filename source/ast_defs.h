#ifndef _AST_DEFS_H
#define _AST_DEFS_H

#include "common.h"
#include "string_view.h"

#define AST_ROOT_NODES_MAX  8192
#define AST_BLOCK_NODES_MAX 4096
#define AST_PROCEDURE_PARAMS_MAX 128

#define _AST_KIND_ALL\
    AST_KIND(AST_Root)\
    AST_KIND(AST_Type_Def)\
    AST_KIND(AST_Parameter)\
    AST_KIND(AST_Declaration)\
    AST_KIND(AST_Block)\
    AST_KIND(AST_Procedure)\
    AST_KIND(AST_Return)\
    AST_KIND(AST_Literal)\
    AST_KIND(AST_Binary)\
    AST_KIND(AST_Variable_Ref)\
    AST_KIND(AST_Procedure_Call)

#define AST_KIND(T) AST_KIND__##T,
typedef enum : uint8_t { _AST_KIND_ALL AST_KIND__COUNT, AST_KIND__INVALID } AST_Kind;
#undef AST_KIND

#define AST_KIND(T) L#T,
static const wchar_t *ast_kind_strings[AST_KIND__COUNT] = { _AST_KIND_ALL };
#undef AST_KIND

// Get AST_Kind enum from AST struct
#define ast_kind(T) AST_KIND__##T

typedef enum : uint16_t {
    TEMP
} AST_Flags;

typedef struct {
    AST_Kind kind;
    AST_Flags flags;
} AST_Node;

typedef struct {
    AST_Node node;
    AST_Node *nodes[AST_ROOT_NODES_MAX];
    size_t nodes_count;
} AST_Root;

typedef enum : uint8_t {
    TYPE_VOID,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_FLOAT64,
    TYPE_CUSTOM
} Type_Kind;

inline const int32_t get_size_of_type(Type_Kind kind) {
    switch(kind) {
        default: {
            assert(0 && "Undefined Type_Kind");
            return -1;
        }

        case TYPE_VOID: {
            return 0;
        }

        case TYPE_INT64:
        case TYPE_UINT64:
        case TYPE_FLOAT64: {
            return 8;
        }

        case TYPE_CUSTOM: {
            assert(0 && "Not implemented @TODO");
        }
    }

    return -1;
}

typedef struct {
    AST_Node node;
    Type_Kind kind;
    Str_View signature;
} AST_Type_Def;

typedef struct {
    AST_Node node;
    Str_View identifier;
    AST_Type_Def *data_type;
} AST_Parameter;

typedef struct {
    AST_Node node;
    Str_View identifier;
    AST_Type_Def *data_type;
    AST_Node *expression;
} AST_Declaration;

typedef struct {
    AST_Node node;
    AST_Node *nodes[AST_BLOCK_NODES_MAX];
    size_t nodes_count;
} AST_Block;

typedef struct {
    AST_Node node;
    Str_View signature;
    AST_Type_Def *return_type;
    AST_Parameter *params[AST_PROCEDURE_PARAMS_MAX];
    size_t params_count;
    AST_Block *block;
} AST_Procedure;

typedef struct {
    AST_Node node;
    AST_Node *expression;
} AST_Return;

typedef enum : uint8_t {
    LITERAL_INT64 = 0,
    LITERAL_UINT64,
    LITERAL_FLOAT64,
    LITERAL__INVALID,
} Literal_Kind;

typedef struct {
    AST_Node node;
    Literal_Kind kind;
    union {
        int64_t  value_int64;
        uint64_t value_uint64;
        double   value_float64;
    };
} AST_Literal;

typedef enum : uint8_t {
    BINARY_OP_ADD = 0,
    BINARY_OP_SUB,
    BINARY_OP_MUL,
    BINARY_OP_DIV,
    BINARY_OP__COUNT
} Binary_Operation;

inline const wchar_t *binary_operation_string(Binary_Operation op) {
    switch(op) {
        default: assert(0 && "Unhandled Binary_Operation in binary_operation_string");
        case BINARY_OP_ADD: return L"+";
        case BINARY_OP_SUB: return L"-";
        case BINARY_OP_MUL: return L"*";
        case BINARY_OP_DIV: return L"/";
    }
}

typedef struct {
    AST_Node node;
    AST_Node *expr_l;
    AST_Node *expr_r;
    Binary_Operation operation;
} AST_Binary;

typedef struct {
    AST_Node node;
    Str_View var_ident;
} AST_Variable_Ref;

typedef struct {
    AST_Node node;
    Str_View procedure_signature;
    AST_Node *params[AST_PROCEDURE_PARAMS_MAX];
    size_t params_count;
} AST_Procedure_Call;


#endif /* _AST_DEFS_H */
