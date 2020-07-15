
#include "global.h"
#include "symtab.h"
#include "analyze.h"

static void printError(SyntaxTree newNode, Bucket bucket, ErrorType type)
{
    errFlag = 1;
    switch (type) {

    case Redefinition:
        printf("[Duplicate identifier]: Redefinition of '%s' in line %d. Previous definition is in line %d.\n", newNode->attr.name, newNode->lineno, bucket->lineno);
    case Undefinition:
        printf("[Unknown identifier]: Use of undeclared identifier '%s' in line %d.\n", newNode->attr.name, newNode->lineno);
        break;
    case ConstModification:
        printf("[Illegal assignment]: Cannot assign to variable '%s' in line %d with const-qualified type.\n", newNode->attr.name, newNode->lineno);
        break;
    case FuncUndefinition:
        printf("[Unknown symbol]: Use of undeclared function '%s' in line %d.\n", newNode->attr.name, newNode->lineno);
        break;
    case ArgumentNumber:
        printf("[Argument unmatch]: Illegal arguments to function call '%s' in line %d, expected %d.\n", bucket->name, bucket->lineno, bucket->function->paramCount);
        break;
    case ErrorInType:
        printf("[Error in type]: Error type in line %d.\n", newNode->lineno);
        break;
    case TypeMismatch:
        printf("[Type mismatch]: Type mismatch in line %d.\n", newNode->lineno);
        break;
    default:
        break;
    }
}

void buildSymtab(SyntaxTree treeNode, Symtab symtab)
{
    Bucket bucket, temp;
    SyntaxTree nodeTemp;
    char* charTemp;
    if (treeNode == NULL)
        return;
    switch (treeNode->nodeKind) {
    case StatementNode:
        nodeTemp = treeNode;
        while (nodeTemp != NULL) {
            semanticAnalysis(nodeTemp, symtab, 0);
            nodeTemp = nodeTemp->sibling;
        }
        break;
    case ExpressionNode:
        break;
    case DeclareNode:
        switch (treeNode->kind.decl) {

        case ProgramDecl:
            symtab = SymtabCreate(treeNode->attr.name, symtab);
            dummySymtab = symtab;
            buildSymtab(treeNode->child[0], symtab);
            buildSymtab(treeNode->child[1], symtab);
            break;

        case RoutineDecl:
            break;

        case RoutineheadDecl:
            for (int i = 0; i < MAXCHILDREN; i++) {
                buildSymtab(treeNode->child[i], symtab);
            }
            break;

        case FunctionDecl:
            temp = SymtabSearch(symtab, treeNode->attr.name);
            if (temp != NULL) {
                printError(treeNode, temp, Redefinition);
                break;
            }
            bucket = createFuncBucket(treeNode, treeNode->child[1], treeNode->child[3]);
            nodeTemp = treeNode->child[0];
            while (nodeTemp != NULL) {
                setFunctionParameter(bucket, nodeTemp->child[0], nodeTemp->child[1]);
                nodeTemp = nodeTemp->sibling;
            }
            SymtabInsert(symtab, bucket);
            symtab = SymtabCreate(treeNode->attr.name, symtab);
            buildSymtab(treeNode->child[0], symtab);
            buildSymtab(treeNode->child[2], symtab);
            buildSymtab(treeNode->child[3], symtab);
            break;

        case ProcedureDecl:
            temp = SymtabSearch(symtab, treeNode->attr.name);
            if (temp != NULL) {
                printError(treeNode, temp, Redefinition);
                break;
            }
            bucket = createFuncBucket(treeNode, treeNode->child[1], treeNode->child[3]);
            nodeTemp = treeNode->child[0];
            while (nodeTemp != NULL) {
                setFunctionParameter(bucket, nodeTemp->child[0], nodeTemp->child[1]);
                nodeTemp = nodeTemp->sibling;
            }
            SymtabInsert(symtab, bucket);
            symtab = SymtabCreate(treeNode->attr.name, symtab);
            buildSymtab(treeNode->child[0], symtab);
            buildSymtab(treeNode->child[2], symtab);
            buildSymtab(treeNode->child[3], symtab);
            break;

        case ConstDecl:
            temp = CurrentSymtabSearch(symtab, treeNode->attr.name);
            if (temp != NULL) {
                printError(treeNode->child[0], temp, Redefinition);
                break;
            }
            bucket = createParamBucket(treeNode->child[0], treeNode->child[1]);
            SymtabInsert(symtab, bucket);
            charTemp = copyString("-");
            strcat(charTemp, symtab->nameSpace);
            strcat(treeNode->child[0]->attr.name, charTemp);
            buildSymtab(treeNode->sibling, symtab);
            break;

        case VarDecl:
            nodeTemp = treeNode->child[0];
            do {
                temp = CurrentSymtabSearch(symtab, nodeTemp->attr.name);
                if (temp != NULL) {
                    printError(nodeTemp, temp, Redefinition);
                    break;
                }
                bucket = createParamBucket(nodeTemp, treeNode->child[1]);
                SymtabInsert(symtab, bucket);
                charTemp = copyString("-");
                strcat(charTemp, symtab->nameSpace);
                strcat(nodeTemp->attr.name, charTemp);
                nodeTemp = nodeTemp->sibling;
            } while (nodeTemp != NULL);
            buildSymtab(treeNode->sibling, symtab);
            break;

        case TypeDecl:
            bucket = createParamBucket(treeNode->child[0], treeNode->child[1]);
            //                buildSymtab(treeNode->child[1], symtab);
            break;

        case VarParaDecl:
            nodeTemp = treeNode->child[0];
            do {
                temp = CurrentSymtabSearch(symtab, nodeTemp->attr.name);
                if (temp != NULL) {
                    printError(nodeTemp, temp, Redefinition);
                    break;
                }
                bucket = createParamBucket(nodeTemp, treeNode->child[1]);
                SymtabInsert(symtab, bucket);
                charTemp = copyString("-");
                strcat(charTemp, symtab->nameSpace);
                strcat(nodeTemp->attr.name, charTemp);
                nodeTemp = nodeTemp->sibling;
            } while (nodeTemp != NULL);
            buildSymtab(treeNode->sibling, symtab);
            break;
        }
        break;

    case TypeNode:
        break;
    }
}

ExpType semanticAnalysis(SyntaxTree treeNode, Symtab symtab, int modification)
{
    Bucket bucket;
    SyntaxTree tempNode;
    ExpType type = UnknowExpType;
    ExpType type1, type2;
    char* charTemp;
    if (treeNode == NULL)
        return type;
    switch (treeNode->nodeKind) {

    case StatementNode:
        switch (treeNode->kind.stmt) {

        case GotoStmt:
            break;
        case LabelStmt:
            semanticAnalysis(treeNode->child[0], symtab, 0);
            break;

        /* two kids */
        case AssignStmt:
            type1 = semanticAnalysis(treeNode->child[0], symtab, 1);
            if (treeNode->child[2] != NULL) {
                /* array type */
                type2 = semanticAnalysis(treeNode->child[1], symtab, 0);
                if (type2 != IntExpType) {
                    printError(treeNode, NULL, TypeMismatch);
                }
                type2 = semanticAnalysis(treeNode->child[2], symtab, 0);
            } else {
                type2 = semanticAnalysis(treeNode->child[1], symtab, 0);
            }
            if (type1 != type2) {
                printError(treeNode, NULL, TypeMismatch);
            }
            type = type1;
            break;

        case WhileStmt:
            semanticAnalysis(treeNode->child[0], symtab, 0);
            tempNode = treeNode->child[1];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            break;

        case RepeatStmt:
            semanticAnalysis(treeNode->child[1], symtab, 0);
            tempNode = treeNode->child[0];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            break;

        case CaseStmt:
            type1 = semanticAnalysis(treeNode->child[0], symtab, 0);
            tempNode = treeNode->child[1];
            while (tempNode != NULL) {
                type2 = semanticAnalysis(tempNode, symtab, 0);
                if (type1 != type2) {
                    printError(tempNode, NULL, TypeMismatch);
                }
                tempNode = tempNode->sibling;
            }
            type = type1;
            break;

        /* three kids */
        case IfStmt:
            semanticAnalysis(treeNode->child[0], symtab, 0);
            tempNode = treeNode->child[1];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            tempNode = treeNode->child[2];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            break;

        /* four kids */
        case ForStmt:
            semanticAnalysis(treeNode->child[0], symtab, 0);
            semanticAnalysis(treeNode->child[1], symtab, 0);
            semanticAnalysis(treeNode->child[2], symtab, 0);
            tempNode = treeNode->child[3];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            break;

        case ProcIdStmt:
            break;

        case ProcSysStmt:
            switch (treeNode->attr.op) {
            /* one kid */
            case ReadOp:
            case WriteOp:
            case WritelnOp:
                semanticAnalysis(treeNode->child[0], symtab, 0);
                break;
            case ToOp:
                break;
            case DowntoOp:
                break;
            }
            break;

        case FuncIdStmt:
            bucket = SymtabSearch(symtab, treeNode->attr.name);
            if (bucket == NULL) {
                printError(treeNode, NULL, FuncUndefinition);
            }
            tempNode = treeNode->child[0];
            while (tempNode != NULL) {
                semanticAnalysis(tempNode, symtab, 0);
                tempNode = tempNode->sibling;
            }
            paramCheck(bucket, treeNode->child[0], symtab);
            type1 = bucket != NULL ? bucket->function->returnType : VoidExpType;
            break;

        case FuncSysStmt:
            type1 = semanticAnalysis(treeNode->child[0], symtab, 0);
            switch (treeNode->attr.op) {
            case AbsOp:
            case SqrOp:
            case SqrtOp:
                if (type1 != IntExpType || type1 != RealExpType) {
                    printError(treeNode->child[0], NULL, TypeMismatch);
                }
                break;
            case OddOp:
            /* 判断一个数是否为奇数 */
            case ChrOp:
                /* 将ascii码转换为字母 */
                if (type1 != IntExpType) {
                    printError(treeNode->child[0], NULL, TypeMismatch);
                }
                break;
            case OrdOp:
                /* 返回字母的ascii码*/
                if (type1 != CharExpType) {
                    printError(treeNode->child[0], NULL, TypeMismatch);
                }
                break;
            case PredOp:
            /* 求前驱 */
            case SuccOp:
                /* 求后继 */
                if (type1 != IntExpType || type1 != CharExpType || type1 != BoolExpType) {
                    printError(treeNode->child[0], NULL, TypeMismatch);
                }
                break;
            }
            break;
        }
        type = type1;
        break;

    case ExpressionNode:
        switch (treeNode->kind.exp) {

        case IdExp:
            bucket = SymtabSearch(symtab, treeNode->attr.name);
            if (bucket == NULL) {
                printError(treeNode, NULL, Undefinition);
            } else if (bucket->type == ParamType && modification == 1 && bucket->parameter->modification != 1) {
                printError(treeNode, NULL, ConstModification);
            }
            if (bucket != NULL) {
                if (bucket->type == ParamType) {
                    if (bucket->parameter->type == ArrayExpType) {
                        type = bucket->parameter->next->type;
                    } else {
                        type = bucket->parameter->type;
                    }
                    charTemp = copyString("-");
                    strcat(charTemp, SymtabSearchName(symtab, treeNode->attr.name));
                    strcat(treeNode->attr.name, charTemp);
                } else {
                    type = bucket->function->returnType;
                }
            }
            break;
        case ConstExp:
            type = treeNode->type;
            break;
        case OpExp:
            switch (treeNode->attr.op) {
            case GeOp:
            case GtOp:
            case LeOp:
            case LtOp:
            case EqualOp:
            case UnequalOp:
            case PlusOp:
            case MinusOp:
            case MulOp:
            case DivOp:
            case ModOp:
                type1 = semanticAnalysis(treeNode->child[0], symtab, 0);
                type2 = semanticAnalysis(treeNode->child[1], symtab, 0);
                if (type1 != type2) {
                    printError(treeNode, NULL, TypeMismatch);
                } else {
                    if (treeNode->child[0]->kind.exp == ConstExp && treeNode->child[1]->kind.exp == ConstExp) {
                        tempNode = newExpNode(ConstExp);
                        tempNode->type = type1;
                        switch (type1) {
                        case IntExpType:
                            switch (treeNode->attr.op) {
                            case PlusOp:
                                tempNode->attr.intValue = treeNode->child[0]->attr.intValue + treeNode->child[1]->attr.intValue;
                                break;
                            case MinusOp:
                                tempNode->attr.intValue = treeNode->child[0]->attr.intValue - treeNode->child[1]->attr.intValue;
                                break;
                            case MulOp:
                                tempNode->attr.intValue = treeNode->child[0]->attr.intValue * treeNode->child[1]->attr.intValue;
                                break;
                            case DivOp:
                                tempNode->attr.intValue = treeNode->child[0]->attr.intValue / treeNode->child[1]->attr.intValue;
                                break;
                            case ModOp:
                                tempNode->attr.intValue = treeNode->child[0]->attr.intValue % treeNode->child[1]->attr.intValue;
                                break;
                            default:
                                break;
                            }
                            break;
                        case RealExpType:
                            switch (treeNode->attr.op) {
                            case PlusOp:
                                treeNode->attr.realValue = treeNode->child[0]->attr.realValue + treeNode->child[1]->attr.realValue;
                                break;
                            case MinusOp:
                                treeNode->attr.realValue = treeNode->child[0]->attr.realValue - treeNode->child[1]->attr.realValue;
                                break;
                            case MulOp:
                                treeNode->attr.realValue = treeNode->child[0]->attr.realValue * treeNode->child[1]->attr.realValue;
                                break;
                            case DivOp:
                                treeNode->attr.realValue = treeNode->child[0]->attr.realValue / treeNode->child[1]->attr.realValue;
                                break;
                            default:
                                break;
                            }
                            break;
                        case StringExpType:
                            switch (treeNode->attr.op) {
                            case PlusOp:
                                tempNode->attr.stringValue = copyString(treeNode->child[0]->attr.stringValue);
                                strcat(tempNode->attr.stringValue, treeNode->child[1]->attr.stringValue);
                                break;
                            default:
                                break;
                            }
                            break;
                        }
                        if (type1 == IntExpType || type1 == RealExpType || type1 == StringExpType) {
                            treeNode->kind = tempNode->kind;
                            treeNode->type = tempNode->type;
                            switch (type1) {
                            case IntExpType:
                                treeNode->attr.intValue = tempNode->attr.intValue;
                                break;
                            case RealExpType:
                                treeNode->attr.realValue = tempNode->attr.realValue;
                                break;
                            case StringExpType:
                                treeNode->attr.stringValue = tempNode->attr.stringValue;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
                type = type1;
                break;
            case NotOp:
                semanticAnalysis(treeNode->child[0], symtab, 0);
                break;
            case AndOp:
            case OrOp:
                break;
            }
            break;
        case CaseExp:
            type1 = semanticAnalysis(treeNode->child[0], symtab, 0);
            semanticAnalysis(treeNode->child[1], symtab, 0);
            type = type1;
            break;
        }
        break;

    case DeclareNode:
        break;
    case TypeNode:
        break;
    }
    return type;
}

void paramCheck(Bucket funcBucket, SyntaxTree paramNode, Symtab symtab)
{
    SyntaxTree paramTemp = paramNode;
    if (funcBucket == NULL)
        return;
    int count = 0;
    while (paramTemp != NULL) {
        count++;
        paramTemp = paramTemp->sibling;
    }
    if (count != funcBucket->function->paramCount) {
        printError(NULL, funcBucket, ArgumentNumber);
        return;
    }
}
