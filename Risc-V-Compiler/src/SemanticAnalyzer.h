#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "Common.h"
#include "SyntaxAnalyzer.h"
#include "ASMGenerator.h"
#include "SpecialParser.h"
#define NULL_DEPTH 255

uint8_t GetPointerDepth(TreeNode *parent)
{
    uint8_t count = 0;
    TreeNode *currentNode = parent;
    while (currentNode->children.count > 0)
    {
        count++;
        currentNode = (TreeNode *)ListGetIndex(&currentNode->children, 0);
    }
    count--;
    debugPrintf("Pointer depth: %d\n", (int)count);
    return count;
}

Identifier *FindSymbol(GeneralList *variableStack, char *symbol)
{
    bool isError = true;
    for (int i = 0; i < variableStack->count; i++)
    {
        Identifier *checkSymbol = (Identifier *)ListGetIndex(variableStack, i);
        debugPrintf("comparing %s to symbol: %s\n", symbol, checkSymbol->name);
        if (strcmp(checkSymbol->name, symbol) == 0)
        {
            debugPrintf("Found matching symbol, no error\n");
            return checkSymbol;
        }
    }
    debugPrintf("not found\n");
    return NULL;
}
// Determines the current index of a node in relation to its siblings
uint8_t GetIndexFromSiblings(TreeNode *self)
{
    TreeNode *parent = self->parent;
    for (int i = 0; i < parent->children.count; i++)
    {
        TreeNode *checkSibling = (TreeNode *)ListGetIndex(&parent->children, i);
        if (checkSibling == self)
        {
            return i;
        }
    }
    return 0;
}

void BacktrackTypeCheck(GeneralList *mathStack, uint8_t *nextReg, Error *error, GeneralList *typeMarkers, TreeNode *currentNode);
void LeftMath(GeneralList *mathStack, uint8_t *nextReg, Error *error, GeneralList *typeMarkers, TreeNode *currentNode);

bool IsFunctionDecleration(TreeNode *decleration)
{
    TreeNode *decrest = (TreeNode *)ListGetIndex(&decleration->children, 0);
    TreeNode *funcOrVar = (TreeNode *)PeekList(&decrest->children);
    if ((uint8_t)((Token *)funcOrVar->content)->value[0] == FUNC_DEC_REST + TERM_OFFSET)
    {
        return true;
    }
    else
    {
        return false;
    }
}
Identifier *DeclareVariable(Error *error, GeneralList *variableStack, TreeNode *currentNode, bool forceVariable)
{

    TreeNode *typeNode = (TreeNode *)ListGetIndex(&currentNode->children, 4);
    typeNode = (TreeNode *)ListGetIndex(&typeNode->children, 0);

    TreeNode *nameNode = (TreeNode *)ListGetIndex(&currentNode->children, 2);
    nameNode = (TreeNode *)ListGetIndex(&nameNode->children, 0);

    debugPrintf("a:\n");
    delay(100);
    if (FindSymbol(variableStack, ((Token *)nameNode->content)->value) != NULL)
    {
        SetError(error, DUPLICATE, SEMANTIC_ANALYZER, "Symbol was already defined earlier", 0);
        return NULL;
    }

    debugPrintf("a1:\n");
    delay(100);
    Identifier *newVar = (Identifier *)malloc(sizeof(Identifier));
    if (newVar != NULL)
    {
        debugPrintf("created newVar\n");
    }
    else
    {
        debugPrintf("failed newVar\n");
    }
    debugPrintf("a2");
    delay(100);

    newVar->name = malloc(strlen(((Token *)nameNode->content)->value) + 1);
    debugPrintf("a3");
    strcpy(newVar->name, ((Token *)nameNode->content)->value);

    debugPrintf("b:\n");
    delay(100);
    newVar->type = ParseType(((Token *)typeNode->content)->value);

    TreeNode *pointerNode = (TreeNode *)ListGetIndex(&currentNode->children, 3);

    newVar->pointerDepth = GetPointerDepth(pointerNode);

    debugPrintf("c:\n");
    delay(100);
    if (forceVariable)
    {
        newVar->funcOrVar = VARIABLE;
    }
    else
    {
        if (IsFunctionDecleration(currentNode))
        {
            debugPrintf("Original node type: %d\n", (int)((Token *)currentNode->content)->type);
            delay(60);
            newVar->funcOrVar = FUNCTION;
            InitializeList(&newVar->parameters);
            TreeNode *parameterExplorer = TreeGetNodeConstant(currentNode, (uint8_t[]){0, 0, 4}, 3);
            debugPrintf("Finding function params\n");
            while (parameterExplorer->children.count > 1)
            {

                debugPrintf("param detected, explorer node type: %d\n", (int)((Token *)parameterExplorer->content)->type);
                delay(60);
                Identifier *newParam = (Identifier *)malloc(sizeof(Identifier));
                char *paramName = ((Token *)TreeGetNodeConstant(parameterExplorer, (uint8_t[]){2, 0}, 2)->content)->value;
                debugPrintf("param name %s\n", paramName);
                newParam->name = malloc(strlen(paramName) + 1);
                strcpy(newParam->name, paramName);
                newParam->pointerDepth = GetPointerDepth(TreeGetNodeConstant(parameterExplorer, (uint8_t[]){3}, 1));
                newParam->funcOrVar = VARIABLE;
                newParam->type = ParseType(((Token *)TreeGetNodeConstant(parameterExplorer, (uint8_t[]){4, 0}, 2)->content)->value);
                PushList(&newVar->parameters, newParam);
                debugPrintf("Defined param %s\n", paramName);

                TreeNode *paramRest = TreeGetNodeConstant(parameterExplorer, (uint8_t[]){0}, 1);
                if (paramRest->children.count <= 1)
                {
                    break;
                }
                parameterExplorer = TreeGetNodeConstant(paramRest, (uint8_t[]){0}, 1);
            }
        }
        else
        {
            newVar->funcOrVar = VARIABLE;
        }
    }
    debugPrintf("Pushing:\n");
    delay(100);
    PushList(variableStack, newVar);
    debugPrintf("pushed %s to symbol table\n", newVar->name);
    delay(100);
    return newVar;
}
void EvaluateExpression(ASMProgram *program, Error *error, TreeNode *currentNode, GeneralList *variableStack)
{
    GeneralList postfix;
    InitializeList(&postfix);
    GeneralList inputTokens;
    InitializeList(&inputTokens);
    while (currentNode->children.count > 0)
    {
        Token *token = (Token *)((TreeNode *)PopListFirst(&currentNode->children))->content;
        debugPrintf("Shunt yard in: %s\n", token->value);
        PushList(&inputTokens, token);
    }
    ShuntYard(error, &inputTokens, variableStack, &postfix);
    if (error->errorCode != NO_ERROR)
    {
        debugPrintf("Shunt yard error within evaluation\n");
        return;
    }
    PostfixToASM(program, error, &postfix, variableStack);
    if (error->errorCode != NO_ERROR)
    {
        return;
    }
    AppendTree(currentNode, PopList(&postfix));
}
void InitializeStdFunctions(GeneralList *variableStack)
{
    Identifier *stdFunction = NULL;
    Identifier *parameter = NULL;

    // Malloc
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = MALLOC_LABEL;
    stdFunction->name = (char *)malloc(strlen("malloc") + 1);
    strcpy(stdFunction->name, "malloc");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("bytes") + 1);
    strcpy(parameter->name, "bytes");
    parameter->pointerDepth = 0;
    parameter->type = UINT32_T;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 1;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // Free
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = FREE_LABEL;
    stdFunction->name = malloc(strlen("free") + 1);
    strcpy(stdFunction->name, "free");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("pointer") + 1);
    strcpy(parameter->name, "pointer");
    parameter->pointerDepth = 1;
    parameter->type = VOID;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // pinMode
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = PIN_MODE;
    stdFunction->name = malloc(strlen("pinMode") + 1);
    strcpy(stdFunction->name, "pinMode");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("pin") + 1);
    strcpy(parameter->name, "pin");
    parameter->pointerDepth = 0;
    parameter->type = UINT8_T;
    PushList(&stdFunction->parameters, parameter);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("output") + 1);
    strcpy(parameter->name, "output");
    parameter->pointerDepth = 0;
    parameter->type = BOOL;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // pinToggle
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = PIN_TOGGLE;
    stdFunction->name = malloc(strlen("pinToggle") + 1);
    strcpy(stdFunction->name, "pinToggle");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("pin") + 1);
    strcpy(parameter->name, "pin");
    parameter->pointerDepth = 0;
    parameter->type = UINT8_T;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // pinWrite
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = PIN_WRITE;
    stdFunction->name = malloc(strlen("pinWrite") + 1);
    strcpy(stdFunction->name, "pinWrite");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("pin") + 1);
    strcpy(parameter->name, "pin");
    parameter->pointerDepth = 0;
    parameter->type = UINT8_T;
    PushList(&stdFunction->parameters, parameter);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("value") + 1);
    strcpy(parameter->name, "value");
    parameter->pointerDepth = 0;
    parameter->type = BOOL;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // pinRead
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = PIN_READ;
    stdFunction->name = malloc(strlen("pinRead") + 1);
    strcpy(stdFunction->name, "pinRead");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("pin") + 1);
    strcpy(parameter->name, "pin");
    parameter->pointerDepth = 0;
    parameter->type = UINT8_T;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = BOOL;
    PushList(variableStack, stdFunction);

    // malloc
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = MALLOC_LABEL;
    stdFunction->name = malloc(strlen("malloc") + 1);
    strcpy(stdFunction->name, "malloc");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("bytes") + 1);
    strcpy(parameter->name, "bytes");
    parameter->pointerDepth = 0;
    parameter->type = UINT32_T;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 1;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);

    // memWrite
    stdFunction = (Identifier *)malloc(sizeof(Identifier));
    stdFunction->funcOrVar = FUNCTION;
    stdFunction->labelNumber = MEM_WRITE;
    stdFunction->name = malloc(strlen("memWrite") + 1);
    strcpy(stdFunction->name, "memWrite");
    InitializeList(&stdFunction->parameters);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("address") + 1);
    strcpy(parameter->name, "address");
    parameter->pointerDepth = 0;
    parameter->type = UINT32_T;
    PushList(&stdFunction->parameters, parameter);

    parameter = (Identifier *)malloc(sizeof(Identifier));
    parameter->funcOrVar = VARIABLE;
    parameter->name = (char *)malloc(strlen("value") + 1);
    strcpy(parameter->name, "value");
    parameter->pointerDepth = 0;
    parameter->type = UINT32_T;
    PushList(&stdFunction->parameters, parameter);

    stdFunction->pointerDepth = 0;
    stdFunction->type = VOID;
    PushList(variableStack, stdFunction);
}
Identifier* CopyIdentifier(Identifier*source){
    Identifier* newId = (Identifier*)malloc(sizeof(Identifier));
    newId->funcOrVar = source->funcOrVar;
    newId->labelNumber = source->labelNumber;
    newId->name = malloc(strlen(source->name)+1);
    strcpy(newId->name, source->name);
    InitializeList(&newId->parameters);
    CpyList(&newId->parameters, &source->parameters, sizeof(Identifier));
    newId->pointerDepth = source->pointerDepth;
    newId->type = source->type;
    return newId;
}   
Error *SemanticAnylsis(ASMProgram *program, TreeNode *AST)
{
    debugPrintf("semantic analysis\n");
    Error *error = (Error *)malloc(sizeof(Error));
    memset(error, 0, sizeof(Error));
    error->errorCode = NO_ERROR;

    debugPrintf("Malloced error done\n");

    if (AST == NULL)
    {
        SetError(error, UNEXPECTED_NULL, SEMANTIC_ANALYZER, "Missing AST reference", 0);
        goto exit;
    }

    GeneralList exploreMap;
    InitializeList(&exploreMap);

    GeneralList variableStack;
    InitializeList(&variableStack);
    debugPrintf("sem init 1\n");
    delay(100);

    InitializeStdFunctions(&variableStack);
    debugPrintf("sem init 2\n");
    delay(100);

    GeneralList tableIndexices;
    InitializeList(&tableIndexices);
    debugPrintf("sem init 3\n");
    delay(100);

    GeneralList loopStack;
    InitializeList(&loopStack);

    TreeNode *currentNode = AST;

    debugPrintf("Initialized semantic analysis\n");
    delay(100);

    // go to first node that is not root.
    if (AST->children.count == 0)
    {
        goto exit;
    }
    PushByte(&exploreMap, currentNode->children.count - 1);
    currentNode = (TreeNode *)PeekList(&currentNode->children);

    while (true)
    {
        for (int i = 0; i < program->scopes.count; i++)
        {
            debugPrintf("Scope type: %d\n", (int)((Scope *)ListGetIndex(&program->scopes, i))->scopeType);
        }
        debugPrintf("semantic loop, map: ");
        for (int i = 0; i < exploreMap.count; i++)
        {
            debugPrintf("%d", (int)*((uint8_t *)ListGetIndex(&exploreMap, i)));
        }
        debugPrintf("\n");

        delay(60);

        char *value = ((Token *)currentNode->content)->value;
        uint16_t lineNumber = ((Token *)currentNode->content)->lineNumber;
        if (value == NULL || value[0] == '\0')
        {
            // Reached epsilon, backtrack
            // Seems odd, probably due to a bad CFG
            debugPrintf("epsilon value, backtracking. May need to fix CFG\n");
            goto backtrack;
        }

        if (*((uint8_t *)value) == TERM_OFFSET + DECLERATION && IsFunctionDecleration(currentNode))
        {
            debugPrintf("declaring function\n");
            Identifier *function = DeclareVariable(error, &variableStack, currentNode, false);
            if (function == NULL)
            {
                goto exit;
            }

            ScopeOpenContext(program, FUNCTION_SCOPE);
            PushByte(&tableIndexices, variableStack.count);
            ASM_Comment(program, function->name);
            ASM_Label(program, program->currentLabelNum);
            UpdateStackList(program, "PCount", PARAMETER);
            for (int i = 0; i < function->parameters.count; i++)
            {
                Identifier *parameter = (Identifier *)ListGetIndex(&function->parameters, i);
                UpdateStackList(program, parameter->name, PARAMETER);
                Identifier* paramCopy = CopyIdentifier(parameter);
                PushList(&variableStack, paramCopy);
            }
            UpdateStackList(program, "T0", REG_STATE);
            UpdateStackList(program, "T1", REG_STATE);
            UpdateStackList(program, "T2", REG_STATE);
            UpdateStackList(program, "T3", REG_STATE);
            UpdateStackList(program, "T4", REG_STATE);
            UpdateStackList(program, "T5", REG_STATE);
            UpdateStackList(program, "T6", REG_STATE);

            UpdateStackList(program, "RETURN", PARAMETER);
            function->labelNumber = program->currentLabelNum;
            program->currentLabelNum++;

            debugPrintf("Function declared\n");
            delay(100);
        }
        if (*((uint8_t *)value) == TERM_OFFSET + ASSIGN)
        {
            debugPrintf("Assign Or Call\n");
            TreeNode *identifierNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){2, 0}, 2);
            if (identifierNode == NULL)
            {
                SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Symbol does not exist in the current scope", lineNumber);
                goto exit;
            }
            debugPrintf("Want to find symbol: %s\n", ((Token *)identifierNode->content)->value);
            Identifier *identifier = FindSymbol(&variableStack, ((Token *)identifierNode->content)->value);
            if (identifier == NULL)
            {
                SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Symbol does not exist in the current scope", lineNumber);
                goto exit;
            }

            TreeNode *callOrAssignNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0}, 2);
            Token *callOrAssign = (Token *)callOrAssignNode->content;
            if (*((uint8_t *)callOrAssign->value) != CALL + TERM_OFFSET)
            {
                debugPrintf("Is assign, add assignment operator to expression\n");
                TreeNode *expressionNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0, 0}, 3);
                TreeNode *assignmentNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0, 1}, 3);
                char *assignmentOperator = ((Token *)assignmentNode->content)->value;
                if (strlen(assignmentOperator) > 1)
                {
                    debugPrintf("Assign has operator\n");
                    Token *newToken = malloc(sizeof(Token));
                    newToken->lineNumber = lineNumber;
                    newToken->type = OPERATOR;
                    newToken->value = malloc(2);
                    newToken->value[0] = assignmentOperator[0];
                    newToken->value[1] = '\0';
                    AppendTree(expressionNode, newToken);

                    newToken = malloc(sizeof(Token));
                    newToken->lineNumber = lineNumber;
                    newToken->type = IDENTIFIER;
                    newToken->value = malloc(strlen(identifier->name) + 1);
                    strcpy(newToken->value, identifier->name);
                    AppendTree(expressionNode, newToken);
                }
                else
                {
                    debugPrintf("Assign has no operator\n");
                }
            }
        }
        if (*((uint8_t *)value) == TERM_OFFSET + RETURN)
        {
            debugPrintf("closing function\n");
            // TODO: pass expression onto stack
            TreeNode *expressionNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0}, 2);
            if (expressionNode != NULL)
            {
                debugPrintf("expression node is not null\n");
                char *exprTokenValue = ((Token *)expressionNode->content)->value;
                if (*((uint8_t *)exprTokenValue) == TERM_OFFSET + EXPRESSION)
                {
                    debugPrintf("return has expression\n");
                    delay(100);
                    EvaluateExpression(program, error, expressionNode, &variableStack);
                    if (error->errorCode != NO_ERROR)
                    {
                        goto exit;
                    }
                    TreeNode *stackNode = TreeGetNodeConstant(expressionNode, (uint8_t[]){0}, 1);
                    Token *tokenPointer = (Token *)stackNode->content;
                    uint16_t stackPointerIndex = 0;
                    sscanf(tokenPointer->value, "%hu", &stackPointerIndex);
                    uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
                    ASM_StackLoad(program, A0, offset * 4);
                }
                else
                {
                    debugPrintf("nevermind\n");
                }
            }
            else
            {
                debugPrintf("No expr node\n");
            }

            ASM_JumpString(program, "functionReturn");
            CloseScopeContext(program);
            goto backtrack;
        }
        if (*((uint8_t *)value) == TERM_OFFSET + IF)
        {
            debugPrintf("If statement\n");

            TreeNode *expressionNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){5}, 1);
            EvaluateExpression(program, error, expressionNode, &variableStack);
            if (error->errorCode != NO_ERROR)
            {
                goto exit;
            }

            debugPrintf("Reserve label: %d\n", program->currentLabelNum);
            uint32_t label = program->currentLabelNum++;

            Token *tokenPointer = (Token *)((TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){5, 0}, 2))->content;
            uint16_t stackPointerIndex = 0;
            sscanf(tokenPointer->value, "%hu", &stackPointerIndex);
            uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
            ASM_StackLoad(program, T1, offset * 4);
            ASM_Branch(program, "beq", T1, X0, label);

            TreeNode *elseRestNode = TreeGetNodeConstant(currentNode, (uint8_t[]){0, 0}, 2);
            if (elseRestNode != NULL && elseRestNode->children.count > 0)
            {
                // is an else
                //
                // beq -> L0
                //      (body)
                // J L1
                // L0:
                //      (body)
                // L1:
                //

                // If statement must be "tricked" into creating L1 at the end
                // so the beq actually goes to the label made by the else

                debugPrintf("Else statement\n");
                uint32_t endLabel = program->currentLabelNum++;

                TreeNode *elseNode = TreeGetNodeConstant(currentNode, (uint8_t[]){0}, 1);

                uint32_t *labelChild = (uint32_t *)malloc(sizeof(uint32_t));
                *labelChild = label;
                AppendTree(elseNode, labelChild);

                labelChild = (uint32_t *)malloc(sizeof(uint32_t));
                *labelChild = endLabel;
                AppendTree(elseNode, labelChild);

                label = endLabel;
            }
            else
            {
                debugPrintf("no if");
            }

            ScopeOpenContext(program, BRANCH_SCOPE);

            uint32_t *labelChild = (uint32_t *)malloc(sizeof(uint32_t));
            *labelChild = label;
            AppendTree(currentNode, labelChild);

            currentNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){4}, 1);
            PushByte(&exploreMap, 4);
            continue;
        }
        if (*((uint8_t *)value) == TERM_OFFSET + ELSE && currentNode->children.count > 1)
        {
            debugPrintf("Else has %d children\n", currentNode->children.count);
            if (currentNode->children.count != 4)
            {
                debugPrintf("Else missing marker pinned by If\n");
                SetError(error, UNEXPECTED_NULL, SEMANTIC_ANALYZER, "Else missing If statement", lineNumber);
                goto exit;
            }
            TreeNode *endLabelNode = (TreeNode *)PopList(&currentNode->children);
            uint32_t *endLabel = (uint32_t *)endLabelNode->content;

            TreeNode *elseLabelNode = (TreeNode *)PopList(&currentNode->children);
            uint32_t *elseLabel = (uint32_t *)elseLabelNode->content;

            // closes the if statement
            CloseScopeContext(program);

            ASM_Jump(program, *endLabel);
            ASM_Label(program, *elseLabel);

            // opens the else
            ScopeOpenContext(program, BRANCH_SCOPE);
            // is closed by the if (outer)
        }
        if (*((uint8_t *)value) == TERM_OFFSET + WHILE)
        {
            ScopeOpenContext(program, LOOP_SCOPE);
            ScopeOpenContext(program, LOOP_SCOPE);
            debugPrintf("while\n");

            uint32_t toplabel = program->currentLabelNum++;
            ASM_Label(program, toplabel);

            TreeNode *expressionNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){4}, 1);
            EvaluateExpression(program, error, expressionNode, &variableStack);
            if (error->errorCode != NO_ERROR)
            {
                goto exit;
            }

            uint32_t endLabel = program->currentLabelNum++;

            Token *tokenPointer = (Token *)((TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){4, 0}, 2))->content;
            uint16_t stackPointerIndex = 0;
            sscanf(tokenPointer->value, "%hu", &stackPointerIndex);
            uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
            ASM_StackLoad(program, T1, offset * 4);
            CloseScopeContext(program);
            ASM_Branch(program, "beq", T1, X0, endLabel);

            uint32_t *labelChild = (uint32_t *)malloc(sizeof(uint32_t));
            *labelChild = endLabel;
            AppendTree(currentNode, labelChild);

            labelChild = (uint32_t *)malloc(sizeof(uint32_t));
            *labelChild = endLabel;
            PushList(&loopStack, labelChild);

            labelChild = (uint32_t *)malloc(sizeof(uint32_t));
            *labelChild = toplabel;
            AppendTree(currentNode, labelChild);

            currentNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){3}, 1);
            PushByte(&exploreMap, 4);
            continue;
        }
        if (*((uint8_t *)value) == TERM_OFFSET + BREAK)
        {
            if (loopStack.count == 0)
            {
                SetError(error, SYNTAX_ERROR, SEMANTIC_ANALYZER, "No loop to break out of", lineNumber);
                goto exit;
            }
            debugPrintf("Break\n");
            uint16_t closeAmount = 0;
            uint16_t scopeIndex = program->scopes.count - 1;
            Scope *backtrackScope = NULL;
            do
            {
                backtrackScope = (Scope *)ListGetIndex(&program->scopes, scopeIndex);
                debugPrintf("Scope %d has %d bytes", scopeIndex, backtrackScope->symbols.count * 4);
                closeAmount += backtrackScope->symbols.count;
                scopeIndex--;
            } while (backtrackScope->scopeType != LOOP_SCOPE);
            debugPrintf("Backtrack %d bytes\n", (int)closeAmount * 4);

            ASM_Imm(program, T6, closeAmount * 4);
            ASM_Math(program, "sub", SP, T6, SP);

            uint32_t *jumpLabel = (uint32_t *)PeekList(&loopStack);
            ASM_Jump(program, *jumpLabel);
        }
        /*if (*((uint8_t *)value) == TERM_OFFSET + FUNCTION_PARAMS)
        {
            if (currentNode->children.count == 5)
            {
                // find parent to add to their parameters
                TreeNode *parent = currentNode;
                while (currentNode->parent != NULL)
                {
                    parent = parent->parent;
                }
                if (parent == NULL)
                {
                    SetError(error, SYNTAX_ERROR, SEMANTIC_ANALYZER, "How did you do this? This parameter has no parent function?!", lineNumber);
                    goto exit;
                }
                Identifier *parentFunciton = FindSymbol(&variableStack, ((Token *)parent->content)->value);
                if (parentFunciton == NULL)
                {
                    SetError(error, SYNTAX_ERROR, SEMANTIC_ANALYZER, "How did you do this? This parameter has no parent function?!", lineNumber);
                    goto exit;
                }
                debugPrintf("Found parent function, pushing param\n");
                Identifier *variable = DeclareVariable(error, &variableStack, currentNode, true);
                if (variable == NULL)
                {
                    goto exit;
                }
                PushList(&parentFunciton->parameters, variable);
            }
        }*/
        if (*((uint8_t *)value) == TERM_OFFSET + EXPRESSION)
        {
            debugPrintf("Expression node\n");
            EvaluateExpression(program, error, currentNode, &variableStack);
            if (error->errorCode != NO_ERROR)
            {
                debugPrintf("Semantic analyzer seen error at expression node\n");
                goto exit;
            }
        }

        // LeftMath(&mathStack, &nextReg, error, &typeMarkers, currentNode);

        if (currentNode->children.count == 0)
        {
        backtrack:
            uint8_t *topValue = (uint8_t *)PeekList(&exploreMap);
            // Go to the next left node
            while ((*topValue) == 0)
            {
                // Go up
                free(PopList(&exploreMap));
                currentNode = currentNode->parent;
                debugPrintf("exploring UP\n");

                if (exploreMap.count == 0)
                {
                    // Reached root node
                    debugPrintf("tree root found, exitting\n");
                    goto success;
                }
                topValue = (uint8_t *)PeekList(&exploreMap);

                char *value = ((Token *)currentNode->content)->value;
                if (*((uint8_t *)value) == TERM_OFFSET + DECLERATION && !IsFunctionDecleration(currentNode))
                {
                    debugPrintf("Backtracked to variable\n");
                    Identifier *variable = DeclareVariable(error, &variableStack, currentNode, false);
                    if (variable == NULL)
                    {
                        goto exit;
                    }
                    TreeNode *expression = TreeGetNodeConstant(currentNode, (uint8_t[]){0, 0, 1}, 3);
                    if (expression == NULL)
                    {
                        debugPrintf("assignment is null\n");
                        ASM_StackPush(program, X0);
                    }
                    else
                    {
                        debugPrintf("assignment val is not null\n");
                        if (expression->children.count != 1)
                        {
                            SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Assignment expression has not been simplified", lineNumber);
                            goto exit;
                        }
                        Token *token = (Token *)(((TreeNode *)PeekList(&expression->children))->content);
                        debugPrintf("Type: %d\n", token->type);
                        debugPrintf("Line: %d\n", token->lineNumber);
                        if (token->type != STACK_POINTER_TOKEN)
                        {
                            SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Assignment expression has not been simplified", lineNumber);
                            goto exit;
                        }

                        uint16_t stackPointerIndex = 0;
                        sscanf(token->value, "%hu", &stackPointerIndex);
                        uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
                        ASM_StackLoad(program, T1, offset * 4);

                        ASM_StackPush(program, T1);
                    }
                    UpdateStackList(program, variable->name, STACK_VAR);
                }
                if (*((uint8_t *)value) == TERM_OFFSET + IF)
                {
                    TreeNode *labelNode = (TreeNode *)PopList(&currentNode->children);
                    uint32_t *label = (uint32_t *)labelNode->content;
                    debugPrintf("Retrieved reserved label: %d\n", *label);
                    CloseScopeContext(program);
                    ASM_Label(program, *label);
                    free(label);
                }
                if (*((uint8_t *)value) == TERM_OFFSET + WHILE)
                {
                    TreeNode *topLabelNode = (TreeNode *)PopList(&currentNode->children);
                    TreeNode *endLabelNode = (TreeNode *)PopList(&currentNode->children);
                    uint32_t *topLabel = (uint32_t *)topLabelNode->content;
                    uint32_t *endLabel = (uint32_t *)endLabelNode->content;
                    CloseScopeContext(program);
                    ASM_Jump(program, *topLabel);
                    ASM_Label(program, *endLabel);
                    free(topLabel);
                    free(endLabel);
                    free(PopList(&loopStack));
                }
                if (*((uint8_t *)value) == TERM_OFFSET + ASSIGN)
                {
                    debugPrintf("Assign Or Call\n");
                    TreeNode *identifierNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){2, 0}, 2);
                    if (identifierNode == NULL)
                    {
                        SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Symbol does not exist in the current scope", lineNumber);
                        goto exit;
                    }
                    debugPrintf("Want to find symbol: %s\n", ((Token *)identifierNode->content)->value);
                    Identifier *identifier = FindSymbol(&variableStack, ((Token *)identifierNode->content)->value);
                    if (identifier == NULL)
                    {
                        SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Symbol does not exist in the current scope", lineNumber);
                        goto exit;
                    }

                    TreeNode *callOrAssignNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0}, 2);
                    Token *callOrAssign = (Token *)callOrAssignNode->content;
                    if (*((uint8_t *)callOrAssign->value) == CALL + TERM_OFFSET)
                    {
                        debugPrintf("Is call\n");
                        if (identifier->funcOrVar != FUNCTION)
                        {
                            SetError(error, SYNTAX_ERROR, SEMANTIC_ANALYZER, "Function calls can only be used on functions", lineNumber);
                            goto exit;
                        }
                        debugPrintf("Current stack list\n");
                        for (int x = 0; x < program->scopes.count; x++)
                        {
                            Scope *scope = (Scope *)ListGetIndex(&program->scopes, x);
                            for (int i = 0; i < scope->symbols.count; i++)
                            {
                                StackPointer *sp = (StackPointer *)ListGetIndex(&scope->symbols, i);
                                debugPrintf("%s\n", sp->name);
                            }
                            debugPrintf("scope end\n");
                        }
                        TreeNode *parameterNode = (TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0, 1}, 3);
                        GeneralList parameters;
                        InitializeList(&parameters);
                        while (parameterNode->children.count > 1)
                        {
                            TreeNode *expression = (TreeNode *)TreeGetNodeConstant(parameterNode, (uint8_t[]){1, 0}, 2);
                            debugPrintf("Found a parameter\n");
                            delay(100);
                            PushList(&parameters, expression);
                            parameterNode = (TreeNode *)PeekListFirst(&parameterNode->children);

                            // I dont know
                            // the expression is not solved for light(random())

                            /*
                            EvaluateExpression(error, currentNode, &variableStack);
            if (error->errorCode != NO_ERROR)
            {
                goto exit;
            }
                            */
                        }
                        debugPrintf("All parameters found");
                        delay(100);
                        ASM_Imm(program, S0, parameters.count * 4 + 4);
                        ASM_Move(program, S0, A2);
                        ASM_JumpStringAndLink(program, S2, "push");

                        for (int i = 0; i < parameters.count; i++)
                        {
                            TreeNode *expressionNode = (TreeNode *)ListGetIndex(&parameters, i);

                            Token *tokenPointer = (Token *)expressionNode->content;
                            uint16_t stackPointerIndex = 0;
                            sscanf(tokenPointer->value, "%hu", &stackPointerIndex);
                            uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
                            ASM_StackLoad(program, S0, offset * 4);

                            ASM_JumpStringAndLink(program, S2, "push");
                        }

                        CallFunction(program, identifier);
                    }
                    else
                    {
                        debugPrintf("Is assign\n");
                        Token *tokenPointer = (Token *)((TreeNode *)TreeGetNodeConstant(currentNode, (uint8_t[]){1, 0, 0, 0}, 4))->content;
                        uint16_t stackPointerIndex = 0;
                        sscanf(tokenPointer->value, "%hu", &stackPointerIndex);
                        uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
                        ASM_StackLoad(program, T1, offset * 4);
                        StackPointer *variableLocation = NULL;
                        bool isGlobalScope = false;
                        for (int i = 0; i < program->scopes.count; i++)
                        {
                            Scope *scope = (Scope *)ListGetIndex(&program->scopes, i);
                            for (int x = 0; x < scope->symbols.count; x++)
                            {
                                StackPointer *stackPointer = (StackPointer *)ListGetIndex(&scope->symbols, x);
                                if (strcmp(stackPointer->name, identifier->name) == 0)
                                {
                                    if (i == 0)
                                    {
                                        isGlobalScope = true;
                                    }
                                    variableLocation = stackPointer;
                                    break;
                                }
                            }
                        }
                        if (variableLocation == NULL)
                        {
                            SetError(error, UNDEFINED, SEMANTIC_ANALYZER, "Did not find variable on the stack", lineNumber);
                            goto exit;
                        }
                        if (isGlobalScope)
                            ASM_StackReplaceGlobal(program, T1, variableLocation->offset * 4);
                        else
                            ASM_StackReplace(program, T1, variableLocation->offset * 4);
                    }
                }
            }

            (*topValue)--;
            currentNode = TreeGetNode(AST, &exploreMap);
        }
        else
        {
            PushByte(&exploreMap, currentNode->children.count - 1);
            currentNode = (TreeNode *)PeekList(&currentNode->children);
        }
    }
success:
    for (int i = 0; i < variableStack.count; i++)
    {
        Identifier *variable = (Identifier *)ListGetIndex(&variableStack, i);
        if (strcmp(variable->name, "main") == 0)
        {
            ASM_Imm(program, S0, variable->parameters.count * 4 + 4);
            ASM_Move(program, S0, A2);
            ASM_JumpStringAndLink(program, S2, "push");

            ASM_LoadAddress(program, A1, variable->labelNumber);
            ASM_JumpStringAndLink(program, A7, "callFunction");
        }
    }
exit:
    while (variableStack.count > 0)
    {
        Identifier *identifier = (Identifier *)PopList(&variableStack);
        if (identifier->funcOrVar == FUNCTION)
        {
            while (identifier->parameters.count > 0)
            {
                Identifier *parameter = (Identifier *)PopList(&identifier->parameters);
                free(parameter->name);
                free(parameter);
            }
        }
        free(identifier->name);
        free(identifier);
    }
    EmptyList(&exploreMap);
    EmptyList(&tableIndexices);
    EmptyList(&loopStack);
    return error;
}

#endif