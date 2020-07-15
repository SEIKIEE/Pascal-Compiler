#ifndef symtab_h
#define symtab_h

#include "global.h"

#define GLOBAL_NAMESPACE "program"
#define SIZE 211
#define SHIFT 4
#define NUMBER 2
#define PARAM 0
#define FUNC 1
#define SYMTAB_SIZE 13

typedef enum{
    ParamType,
    FuncType
} BucketType;

typedef struct ParameterNode {
    char* name;
    ExpType type; /* detail type. e.g. int\real\char */
    int modification;
    union {
        int intValue;
        char charValue;
        char* stringValue;
        float realValue;
    } value[NUMBER];
    struct ParameterNode* next;
} * Parameter;

typedef struct FunctionNode {
    int paramCount; /* 参数个数 */
    Parameter paramAddr; /* 参数开始地址，以链表的形式存储 */
    ExpType returnType; /* 返回值类型 */
    SyntaxTree funcOpAddr; /* routine-body */
} * Function;

typedef struct Entry {
    char* name; /* id name or function name */
    int lineno; /* line num in the code */
    BucketType type;  /* 区分是变量定义还是函数定义 */
    Function function;  /* 函数结构 */
    Parameter parameter; /* 参数结构 */
    struct Entry* next;
} * Bucket;

typedef struct SymtabNode {
    char* nameSpace;
    Bucket table[SIZE]; /* 存放内部常量、变量、函数等 */
    SyntaxTree treeAddr;
    struct SymtabNode* parent;
    struct SymtabNode* child;
    struct SymtabNode* next;
} * Symtab;

void SymtabInsert(Symtab symtab, Bucket bucket);
Symtab SymtabCreate(char* nameSpace, Symtab parent);
Bucket SymtabSearch(Symtab symtab, char* name);
Bucket CurrentSymtabSearch(Symtab symtab, char* name);
char* SymtabSearchName(Symtab symtab, char* name);

Parameter createParamNode(SyntaxTree keyNode, SyntaxTree valueNode);
Bucket createParamBucket(SyntaxTree keyNode, SyntaxTree valueNode);
Bucket createFuncBucket(SyntaxTree keyNode, SyntaxTree resNode, SyntaxTree opNode);
void setFunctionParameter(Bucket funcBucket, SyntaxTree argsKeyNode, SyntaxTree argsValueNode);

#endif /* symtab_h */
