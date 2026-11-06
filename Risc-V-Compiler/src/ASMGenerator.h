#ifndef ASM_GENERATOR_H
#define ASM_GENERATOR_H

#include "Common.h"
#include "SemanticAnalyzer.h"
#include "Common/Keywords.h"
#include "ASMTemplate.h"

typedef enum
{
    FUNCTION_SCOPE,
    BRANCH_SCOPE,
    LOOP_SCOPE,
    GLOBAL_SCOPE
} ScopeType;
typedef struct
{
    ScopeType scopeType;
    GeneralList symbols;
} Scope;

typedef struct ASMProgram
{
    GeneralList functionList;
    GeneralList globalList;
    GeneralList scopes;
    uint32_t currentLabelNum;
} ASMProgram;

void PushTargetASMList(ASMProgram *program, char *value)
{
    if (program->scopes.count > 1)
    {
        PushList(&program->functionList, value);
    }
    else
    {
        PushList(&program->globalList, value);
    }
}
void PopTargetASMList(ASMProgram *program)
{
    if (program->scopes.count > 1)
    {
        free(PopList(&program->functionList));
    }
    else
    {
        free(PopList(&program->globalList));
    }
}

///////////////////////////////////
///////////////////////////////////
///////////////////////////////////

void ASM_Comment(ASMProgram *program, char *message)
{
    char *cstr = malloc(1 + strlen(message) + 1);
    sprintf(cstr, "#%s", message);
    PushTargetASMList(program, cstr);
    debugPrintf("asm comment done");
}
void ASM_Math(ASMProgram *program, char *operation, Registers inA, Registers inB, Registers out)
{
    char *cstr = malloc(strlen(operation) + 1 + 2 + 1 + 2 + 1 + 2 + 1);
    sprintf(cstr, "%s %s,%s,%s", operation, registerNames[(uint8_t)out], registerNames[(uint8_t)inA], registerNames[(uint8_t)inB]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm math done\n");
}
void ASM_MathI(ASMProgram *program, char *operation, Registers destination, Registers source, int value)
{
    char *cstr = malloc(strlen(operation) + 1 + 2 + 1 + 2 + 1 + IntLength(value) + 1);
    sprintf(cstr, "%s %s,%s,%d", operation, registerNames[(uint8_t)destination], registerNames[(uint8_t)source], value);
    PushTargetASMList(program, cstr);
    debugPrintf("ASM_MathI done\n");
}
void ASM_Move(ASMProgram *program, Registers source, Registers destination)
{
    char *cstr = malloc(strlen("mv") + 1 + 2 + 1 + 2 + 1);
    sprintf(cstr, "mv %s,%s", registerNames[(uint8_t)destination], registerNames[(uint8_t)source]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm move done\n");
}
void ASM_Imm(ASMProgram *program, Registers destination, uint32_t value)
{
    char *cstr = malloc(strlen("li") + 1 + 2 + 1 + UintLength(value) + 1);
    sprintf(cstr, "li %s,%u", registerNames[(uint8_t)destination], value);
    PushTargetASMList(program, cstr);
    debugPrintf("ASM_Imm done\n");
}
// Takes the location in bytes (not 4 byte blocks)
void ASM_StackLoad(ASMProgram *program, Registers destination, uint16_t frameOffset)
{
    if (frameOffset < 2000)
    {
        char *cstr = malloc(strlen("lw") + 1 + 2 + 1 + IntLength(frameOffset) + 4 + 1);
        sprintf(cstr, "lw %s,%d(t0)", registerNames[(uint8_t)destination], frameOffset);
        PushTargetASMList(program, cstr);
        debugPrintf("asm load small done\n");
        return;
    }
    ASM_Imm(program, T4, frameOffset);
    ASM_Math(program, "add", T4, T0, T3);
    char *cstr = malloc(strlen("lw") + 1 + 2 + 1 + 2 + 4 + 1);
    sprintf(cstr, "lw %s,0(t3)", registerNames[(uint8_t)destination]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm load big done\n");
}
// Takes the location in bytes (not 4 byte blocks)
void ASM_StackLoadGlobal(ASMProgram *program, Registers destination, uint16_t frameOffset)
{
    ASM_Imm(program, T3, STACK_ADDR);
    ASM_Imm(program, T4, frameOffset);
    ASM_Math(program, "add", T3, T4, T3);
    ASM_Math(program, "add", T3, GP, T3);
    char *cstr = malloc(strlen("lw") + 1 + 2 + 1 + 2 + 4 + 1);
    sprintf(cstr, "lw %s,0(t3)", registerNames[(uint8_t)destination]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm load big done\n");
}
// Takes the location in bytes (not 4 byte blocks)
void ASM_StackReplace(ASMProgram *program, Registers source, uint16_t frameOffset)
{
    if (frameOffset < 2000)
    {
        char *cstr = malloc(strlen("sw") + 1 + 2 + 1 + IntLength(frameOffset) + 4 + 1);
        sprintf(cstr, "sw %s,%d(t0)", registerNames[(uint8_t)source], frameOffset);
        ASM_Comment(program, "StackReplace");
        PushTargetASMList(program, cstr);
        debugPrintf("asm replace small done\n");
        return;
    }
    ASM_Imm(program, T4, frameOffset);
    ASM_Math(program, "add", T4, T0, T3);
    char *cstr = malloc(strlen("sw") + 1 + 2 + 1 + 2 + 4 + 1);
    sprintf(cstr, "sw %s,0(t3)", registerNames[(uint8_t)source]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm replace big done\n");
}
// Takes the location in bytes (not 4 byte blocks)
void ASM_StackReplaceGlobal(ASMProgram *program, Registers source, uint16_t frameOffset)
{

    ASM_Imm(program, T3, STACK_ADDR);
    ASM_Imm(program, T4, frameOffset);
    ASM_Math(program, "add", T3, T4, T3);
    ASM_Math(program, "add", T3, GP, T3);

    char *cstr = malloc(strlen("sw") + 1 + 2 + 1 + 2 + 4 + 1);
    sprintf(cstr, "sw %s,0(t3)", registerNames[(uint8_t)source]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm replace big done\n");
}
// Takes the location in bytes (not 4 byte blocks)
void ASM_LoadFromRegister(ASMProgram *program, Registers destination, Registers location)
{
    char *cstr = malloc(strlen("lw") + 1 + 2 + 1 + 2 + 4 + 1);
    sprintf(cstr, "lw %s,0(%s)", registerNames[(uint8_t)destination], registerNames[(uint8_t)location]);
    PushTargetASMList(program, cstr);
    debugPrintf("asm load from reg loc done\n");
}
void ASM_StackPush(ASMProgram *program, Registers valueReg)
{
    char *cstr = malloc(strlen("sw") + 1 + 2 + 1 + 5 + 1);
    sprintf(cstr, "sw %s,0(sp)", registerNames[(uint8_t)valueReg]);
    PushTargetASMList(program, cstr);
    ASM_MathI(program, "addi", SP, SP, 4);

    debugPrintf("asm push done\n");
}
void ASM_Branch(ASMProgram *program, char *branchType, Registers compareA, Registers compareB, uint32_t label)
{
    char *cstr = malloc(strlen(branchType) + 1 + 2 + 1 + 2 + 1 + 1 + UintLength(label) + 1);
    sprintf(cstr, "%s %s,%s,L%d", branchType, registerNames[(uint8_t)compareA], registerNames[(uint8_t)compareB], label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm branch done\n");
}
void ASM_Label(ASMProgram *program, uint32_t label)
{
    char *cstr = malloc(1 + UintLength(label) + 2);
    sprintf(cstr, "L%d:", label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm label done\n");
}
void ASM_Jump(ASMProgram *program, uint32_t label)
{
    char *cstr = malloc(2 + 1 + UintLength(label) + 1);
    sprintf(cstr, "j L%d", label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm jump done\n");
}
void ASM_JumpString(ASMProgram *program, char *label)
{
    char *cstr = malloc(2 + strlen(label) + 1);
    sprintf(cstr, "j %s", label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm jump done\n");
}
void ASM_LoadAddress(ASMProgram *program, Registers targetRegister, uint32_t label)
{
    char *cstr = malloc(2 + 1 + 2 + 1 + 1 + UintLength(label) + 1);
    sprintf(cstr, "la %s,L%d", registerNames[(uint8_t)targetRegister], label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm LA done\n");
}
void ASM_LoadAddressString(ASMProgram *program, Registers targetRegister, char *label)
{
    char *cstr = malloc(2 + 1 + 2 + 1 + strlen(label) + 1);
    sprintf(cstr, "la %s,%s", registerNames[(uint8_t)targetRegister], label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm LA str done\n");
}
void ASM_JumpAndLink(ASMProgram *program, Registers targetRegister, uint32_t label)
{
    char *cstr = malloc(3 + 1 + 2 + 1 + 1 + UintLength(label) + 1);
    sprintf(cstr, "jal %s,L%d", registerNames[(uint8_t)targetRegister], label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm JAL done\n");
}
void ASM_JumpStringAndLink(ASMProgram *program, Registers targetRegister, char *label)
{
    char *cstr = malloc(3 + 1 + 2 + 1 + strlen(label) + 1);
    sprintf(cstr, "jal %s,%s", registerNames[(uint8_t)targetRegister], label);
    PushTargetASMList(program, cstr);
    debugPrintf("asm JSAL done\n");
}
void CallFunction(ASMProgram *program, Identifier *function)
{
    switch (function->labelNumber)
    {
    case PIN_MODE:
        ASM_LoadAddressString(program, A1, "pinMode");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    case PIN_TOGGLE:
        ASM_LoadAddressString(program, A1, "pinToggle");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    case PIN_WRITE:
        ASM_LoadAddressString(program, A1, "pinWrite");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    case PIN_READ:
        ASM_LoadAddressString(program, A1, "pinRead");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    case MALLOC_LABEL:
        ASM_LoadAddressString(program, A1, "malloc");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    case MEM_WRITE:
        ASM_LoadAddressString(program, A1, "memWrite");
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    default:
        ASM_LoadAddress(program, A1, function->labelNumber);
        ASM_JumpStringAndLink(program, A7, "callFunction");
        break;
    }
    return;
}

///////////////////////////////////
///////////////////////////////////
///////////////////////////////////

void PrintASMProgram(ASMProgram *program)
{
    printfAlways("%s\n", picoAsmHeader);
    for (int i = 0; i < program->globalList.count; i++)
    {
        char *value = (char *)ListGetIndex(&program->globalList, i);
        printfAlways("%s\n", value);
    }
    printfAlways("j EXIT\n");
    printfAlways("\n");

    for (int i = 0; i < program->functionList.count; i++)
    {
        char *value = (char *)ListGetIndex(&program->functionList, i);
        printfAlways("%s\n", value);
    }
    printfAlways("%s\n", stdFunctions);
}

void ScopeOpenContext(ASMProgram *program, ScopeType type)
{
    debugPrintf("Open context %d\n", program->scopes.count);
    delay(100);
    Scope *newScope = (Scope *)malloc(sizeof(Scope));
    InitializeList(&newScope->symbols);
    newScope->scopeType = type;
    PushList(&program->scopes, newScope);
}
void CloseScopeContext(ASMProgram *program)
{
    debugPrintf("Close context %d\n", program->scopes.count);
    delay(100);
    debugPrintf("Closing scope\n");
    if (program->scopes.count <= 1)
    {
        debugPrintf("BAD NO CLOSE GLOBAL SCOPE!!");
        return;
    }
    Scope *lastScope = (Scope *)PeekList(&program->scopes);
    debugPrintf("scope type: %d\n", (int)lastScope->scopeType);
    debugPrintf("scope symbols: %d\n", lastScope->symbols.count);
    if (lastScope->scopeType != FUNCTION_SCOPE && lastScope->symbols.count != 0)
    {
        debugPrintf("Not function scope.\n");
        ASM_Imm(program, T6, lastScope->symbols.count * 4);
        ASM_Math(program, "sub", SP, T6, SP);
    }
    PopList(&program->scopes);

    while (lastScope->symbols.count > 0)
    {
        free(PopList(&lastScope->symbols));
    }
    free(lastScope);
}
void UpdateStackList(ASMProgram *program, char *name, StackType type)
{
    // Multiply by 4
    uint32_t newLocation = 0;
    if (program->scopes.count == 1)
    {
        Scope *scope = (Scope *)PeekListFirst(&program->scopes);
        newLocation += scope->symbols.count;
    }
    else
    {
        for (int x = 1; x < program->scopes.count; x++)
        {
            Scope *scope = (Scope *)ListGetIndex(&program->scopes, x);
            newLocation += scope->symbols.count;
        }
    }
    StackPointer *newPointer = malloc(sizeof(StackPointer));
    newPointer->name = malloc(strlen(name) + 1);
    strcpy(newPointer->name, name);
    newPointer->offset = newLocation;
    newPointer->type = type;
    PushList(&((Scope *)PeekList(&program->scopes))->symbols, newPointer);
    debugPrintf("New stack var: %s\n", newPointer->name);
}

ASMProgram *CreateProgram()
{
    ASMProgram *program = (ASMProgram *)malloc(sizeof(ASMProgram));
    InitializeList(&program->functionList);
    InitializeList(&program->globalList);
    InitializeList(&program->scopes);

    program->currentLabelNum = 0;
    ScopeOpenContext(program, GLOBAL_SCOPE);

    return program;
}

///////////////////////////////////
///////////////////////////////////
///////////////////////////////////

StackPointer *LoadParameter(ASMProgram *program, Error *error, Token *parameter, Registers destinationReg, GeneralList *variableStack)
{
    if (parameter->type == IDENTIFIER)
    {
        debugPrintf("param is identifier\n");
        /*Identifier *identifier = NULL;
        for (int i = 0; i < variableStack->count; i++)
        {
            Identifier *checkIdentifier = (Identifier *)ListGetIndex(variableStack, i);
            if (strcmp(checkIdentifier->name, parameter->value) == 0)
            {
                identifier = checkIdentifier;
                break;
            }
        }
        if (identifier->funcOrVar == FUNCTION)
        {
            debugPrintf(" Is function\n");
            for (int i = 0; i < identifier->parameters.count; i++)
            {
            }
            ASM_LoadAddress(A1, identifier->labelNumber);
            ASM_JumpStringAndLink(A7, "callFunction");
            ASM_Move(A0, destinationReg);
        }
        else
        {*/
        debugPrintf(" Is variable: %s\n", parameter->value);
        delay(100);
        StackPointer *stackPointer = NULL;
        bool notFound = true;
        bool isGlobalScope = false;
        for (int x = 0; x < program->scopes.count; x++)
        {
            Scope *scope = ((Scope *)ListGetIndex(&program->scopes, x));
            for (int i = 0; i < scope->symbols.count; i++)
            {
                StackPointer *checkPointer = ListGetIndex(&scope->symbols, i);
                debugPrintf("checking scope: %s\n", checkPointer->name);
                if (strcmp(checkPointer->name, parameter->value) == 0)
                {
                    stackPointer = checkPointer;
                    notFound = false;
                    if (x == 0)
                    {
                        isGlobalScope = true;
                    }
                    debugPrintf("found at stack loc: %d\n", stackPointer->offset);
                    break;
                }
            }
        }
        if (notFound)
        {
            debugPrintf("not found\n");
            delay(100);

            SetError(error, SYNTAX_ERROR, ASM_GENERATION, "Symbol does not exist in current scope", parameter->lineNumber);
            return NULL;
        }
        if (isGlobalScope)
            ASM_StackLoadGlobal(program, destinationReg, stackPointer->offset * 4);
        else
            ASM_StackLoad(program, destinationReg, stackPointer->offset * 4);
        return stackPointer;
        //}
    }
    else if (parameter->type == STACK_POINTER_TOKEN)
    {
        debugPrintf("param is STACK_POINTER_TOKEN\n");
        delay(100);
        uint16_t stackPointerIndex = 0;
        sscanf(parameter->value, "%hu", &stackPointerIndex);
        uint32_t offset = ((StackPointer *)ListGetIndex(&((Scope *)PeekList(&program->scopes))->symbols, stackPointerIndex))->offset;
        ASM_StackLoad(program,destinationReg, offset * 4);
        return NULL;
    }
    else
    {
        debugPrintf("param is literal\n");
        delay(100);
        IdentifierType type = TypeOfString(parameter->value);
        if (type == BOOL)
        {
            debugPrintf("bool lit\n");
            if (strcmp(parameter->value, "false") == 0)
                ASM_MathI(program,"addi", destinationReg, X0, 0);
            else
                ASM_MathI(program,"addi", destinationReg, X0, 1);
        }
        else if (type <= UINT32_T)
        {
            debugPrintf("uint lit\n");
            uint32_t value = 0;
            uint8_t matches = sscanf(parameter->value, "%u", &value);
            if (matches == 0)
            {
                SetError(error, SYNTAX_ERROR, ASM_GENERATION, "uint32_t does not match format", parameter->lineNumber);
            }
            ASM_Imm(program,destinationReg, value);
        }
        else if (type <= INT)
        {
            debugPrintf("int lit\n");
            int value = 0;
            uint8_t matches = sscanf(parameter->value, "%d", &value);
            if (matches == 0)
            {
                SetError(error, SYNTAX_ERROR, ASM_GENERATION, "uint32_t does not match format", parameter->lineNumber);
            }
            ASM_Imm(program,destinationReg, value);
        }
        else if (type == CHAR)
        {
            debugPrintf("str lit\n");

            if (strlen(parameter->value) < 2)
            {
                char value = parameter->value[0];
                ASM_Imm(program,destinationReg, (uint32_t)value);
                return NULL;
            }
            if (parameter->value[0] != '\"' || parameter->value[strlen(parameter->value - 2)] != '\"')
            {
                debugPrintf("str format error\n");

                SetError(error, SYNTAX_ERROR, ASM_GENERATION, "string does not match format", parameter->lineNumber);
                return NULL;
            }
            debugPrintf("Scopes: %d\n", program->scopes.count);
            uint16_t litLoc = ((Scope *)PeekList(&program->scopes))->symbols.count;
            for (int i = 1; i < strlen(parameter->value) - 1; i++)
            {
                debugPrintf("%d\n", i);

                ASM_Imm(program,destinationReg, parameter->value[i]);
                ASM_StackPush(program,destinationReg);
                UpdateStackList(program,"STRLIT", STR_LITERAL);
            }
            ASM_StackPush(program,X0);
            UpdateStackList(program,"STRLIT", STR_LITERAL);
            ASM_Imm(program,destinationReg, (uint32_t)litLoc);
        }
        return NULL;
    }
    debugPrintf("Parameter is none!\n");
    SetError(error, UNEXPECTED_NULL, ASM_GENERATION, "Parameter is invalid", parameter->lineNumber);
    return NULL;
}
void PostfixToASM(ASMProgram*program,Error *error, GeneralList *postfix, GeneralList *variableStack)
{
    uint16_t lineNumber = ((Token *)PeekList(postfix))->lineNumber;
    uint16_t index = 0;
    debugPrintf("Special parser postfix out:\n");
    for (int i = 0; i < postfix->count; i++)
    {
        Token *printToken = (Token *)ListGetIndex(postfix, i);
        debugPrintf("%s", printToken->value);
    }
    debugPrintf("\n");
    if (postfix->count == 1)
    {
        Token *token = (Token *)PeekList(postfix);
        if (token->type == IDENTIFIER)
        {
            Identifier *function = NULL;
            debugPrintf("Single identifier, checking if its a function\n");
            for (int i = 0; i < variableStack->count; i++)
            {
                Identifier *checkIdentifier = (Identifier *)ListGetIndex(variableStack, i);
                if (strcmp(checkIdentifier->name, token->value) == 0)
                {
                    function = checkIdentifier;
                    break;
                }
            }
            if (function->funcOrVar == FUNCTION)
            {
                debugPrintf("Is function, switching to normal parsing\n");
                goto isFunction;
            }
        }
        PopList(postfix);
        LoadParameter(program,error, token, T1, variableStack);
        ASM_StackPush(program,T1);
        UpdateStackList(program,"TEMP", STACK_POINTER);

        Token *newToken = malloc(sizeof(Token));
        newToken->type = STACK_POINTER_TOKEN;
        newToken->lineNumber = lineNumber;

        uint16_t stackIndex = ((Scope *)PeekList(&program->scopes))->symbols.count - 1;
        newToken->value = malloc(IntLength(stackIndex) + 1);
        sprintf(newToken->value, "%hu", stackIndex);
        PushList(postfix, newToken);
    }
    else
    isFunction:
        while (true)
        {
            if (index >= postfix->count)
            {
                debugPrintf("break, %d, %d\n", index, postfix->count);
                break;
            }
            Token *currentToken = (Token *)ListGetIndex(postfix, index);
            debugPrintf("current: %s\n", currentToken->value);
            delay(60);
            if (currentToken->type == IDENTIFIER)
            {
                debugPrintf("Function found\n");
                Identifier *function = NULL;
                for (int i = 0; i < variableStack->count; i++)
                {
                    Identifier *checkIdentifier = (Identifier *)ListGetIndex(variableStack, i);
                    if (strcmp(checkIdentifier->name, currentToken->value) == 0)
                    {
                        function = checkIdentifier;
                        break;
                    }
                }
                if (function->funcOrVar != FUNCTION)
                {
                    index++;
                    debugPrintf("Not a function, skipping\n");
                    continue;
                }
                ASM_Imm(program,S0, function->parameters.count * 4 + 4);
                ASM_Move(program,S0, A2);
                ASM_JumpStringAndLink(program,S2, "push");
                for (int i = 0; i < function->parameters.count; i++)
                {
                    Token *parameter = (Token *)ListGetIndex(postfix, index - function->parameters.count + i);
                    if (parameter == NULL)
                    {
                        debugPrintf("missing param\n");
                        SetError(error, UNEXPECTED_NULL, ASM_GENERATION, "Function missing parameter", lineNumber);
                        goto exit;
                    }
                    debugPrintf("loading param %d...\n", i);
                    StackPointer *stackPointer = LoadParameter(program,error, parameter, S0, variableStack);
                    debugPrintf("loaded\n");
                    delay(100);
                    if (error->errorCode != NO_ERROR)
                    {
                        debugPrintf("load err\n");
                        delay(100);
                        error->line = currentToken->lineNumber;
                        goto exit;
                    }
                    ASM_JumpStringAndLink(program,S2, "push");
                }
                debugPrintf("calling\n");
                delay(100);
                CallFunction(program,function);
                debugPrintf("function called\n");
                delay(100);

                for (int i = 0; i < function->parameters.count + 1; i++)
                {
                    debugPrintf("deleting parameter %d/%d, at index %s\n", i, function->parameters.count + 1, index - function->parameters.count);
                    delay(100);
                    free(DeleteListElement(postfix, index - function->parameters.count));
                }
                ASM_StackPush(program,A0);
                UpdateStackList(program,"RET_VAL", STACK_POINTER);

                Token *newToken = malloc(sizeof(Token));
                newToken->type = STACK_POINTER_TOKEN;
                newToken->lineNumber = currentToken->lineNumber;

                uint16_t stackIndex = ((Scope *)PeekList(&program->scopes))->symbols.count - 1;
                newToken->value = malloc(IntLength(stackIndex) + 1);
                sprintf(newToken->value, "%hu", stackIndex);
                debugPrintf("insert to : %d\n", index - function->parameters.count);
                ListInsert(postfix, index - function->parameters.count, newToken);
                debugPrintf("set index to 0\n");
                index = 0;
                continue;
            }
            else if (currentToken->type == OPERATOR)
            {
                debugPrintf("Operator found\n");
                OperatorPrecedence *operator= NULL;
                for (int i = 0; i < OPERATOR_PRECEDENT_COUNT; i++)
                {
                    if (strcmp(OPERATOR_PRECEDENT_LIST[i].operator, currentToken->value) == 0)
                    {
                        operator= & OPERATOR_PRECEDENT_LIST[i];
                        break;
                    }
                }
                if (operator== NULL)
                {
                    SetError(error, SYNTAX_ERROR, ASM_GENERATION, "Operator not recognized", lineNumber);
                    goto exit;
                }

                Token *parameters[5];
                StackPointer *param1StackLoc = NULL;
                debugPrintf("getting params\n");
                for (int i = 0; i < operator->parameters; i++)
                {
                    parameters[i] = (Token *)ListGetIndex(postfix, index - operator->parameters + i);
                    if (parameters[i] == NULL)
                    {
                        debugPrintf("missing param\n");
                        SetError(error, UNEXPECTED_NULL, ASM_GENERATION, "Operator missing parameter", lineNumber);
                        goto exit;
                    }
                    StackPointer *stackPointer = LoadParameter(program,error, parameters[i], T1 + i, variableStack);
                    if (error->errorCode != NO_ERROR)
                    {
                        error->line = currentToken->lineNumber;
                        goto exit;
                    }
                    if (i == 0)
                    {
                        param1StackLoc = stackPointer;
                    }
                }

                if (operator->parameters == 2)
                {
                    if (strcmp(currentToken->value, "+") == 0)
                    {
                        debugPrintf("Add operator");
                        ASM_Math(program,"add", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "-") == 0)
                    {
                        debugPrintf("Sub operator");
                        ASM_Math(program,"sub", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "*") == 0)
                    {
                        debugPrintf("Mult operator");
                        ASM_Math(program,"mul", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "/") == 0)
                    {
                        debugPrintf("Div operator");
                        ASM_Math(program,"div", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "%") == 0)
                    {
                        debugPrintf("%% operator");
                        ASM_Math(program,"rem", T1, T2, T1);
                    }

                    if (strcmp(currentToken->value, "==") == 0)
                    {
                        debugPrintf("== operator");
                        ASM_Math(program,"xor", T1, T2, T1);
                        ASM_MathI(program,"sltiu", T1, T1, 1);
                    }
                    if (strcmp(currentToken->value, "!=") == 0)
                    {
                        debugPrintf("!= operator");
                        ASM_Math(program,"xor", T1, T2, T1);
                        ASM_Math(program,"sltu", X0, T1, T1);
                    }
                    if (strcmp(currentToken->value, "<") == 0)
                    {
                        debugPrintf("< operator");
                        ASM_Math(program,"slt", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "<=") == 0)
                    {
                        debugPrintf("<= operator");
                        ASM_Math(program,"slt", T1, T2, T3);
                        ASM_Math(program,"xor", T1, T2, T4);
                        ASM_MathI(program,"sltiu", T4, T4, 1);
                        ASM_Math(program,"or", T3, T4, T1);
                    }
                    if (strcmp(currentToken->value, ">") == 0)
                    {
                        debugPrintf("> operator");

                        ASM_Math(program,"slt", T1, T2, T3);
                        ASM_Math(program,"xor", T1, T2, T1);
                        ASM_Math(program,"sltu", X0, T1, T1);
                        ASM_Math(program,"xor", T1, T3, T1);
                    }
                    if (strcmp(currentToken->value, ">=") == 0)
                    {
                        debugPrintf(">= operator");
                        ASM_Math(program,"slt", T1, T2, T1);
                        ASM_MathI(program,"xori", T1, T1, 1);
                    }
                    if (strcmp(currentToken->value, "|") == 0)
                    {
                        debugPrintf("| operator");
                        ASM_Math(program,"or", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "||") == 0)
                    {
                        debugPrintf("|| operator");
                        ASM_Math(program,"sltu", X0, T1, T1);
                        ASM_Math(program,"sltu", X0, T2, T2);
                        ASM_Math(program,"or", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "&") == 0)
                    {
                        debugPrintf("& operator");
                        ASM_Math(program,"and", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "&&") == 0)
                    {
                        debugPrintf("&& operator");
                        ASM_Math(program,"sltu", X0, T1, T1);
                        ASM_Math(program,"sltu", X0, T2, T2);
                        ASM_Math(program,"and", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "^") == 0)
                    {
                        debugPrintf("^ operator");
                        ASM_Math(program,"xor", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "<<") == 0)
                    {
                        debugPrintf("<< operator");
                        ASM_Math(program,"sll", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, ">>") == 0)
                    {
                        debugPrintf("<< operator");
                        ASM_Math(program,"srl", T1, T2, T1);
                    }
                }
                if (operator->parameters == 1)
                {
                    if (strcmp(currentToken->value, "u*") == 0)
                    {
                        debugPrintf("Dereference operator");
                        ASM_Math(program,"add", T1, GP, T1);
                        ASM_MathI(program,"addi", T1, T1, HEAP_ADDR);
                        ASM_LoadFromRegister(program,T1, T1);
                    }
                    if (strcmp(currentToken->value, "u-") == 0)
                    {
                        debugPrintf("unary sub operator");
                        ASM_Math(program,"sub", X0, T1, T1);
                    }
                    if (strcmp(currentToken->value, "u!") == 0)
                    {
                        debugPrintf("Unary not");
                        ASM_Branch(program,"beq", T1, X0, program->currentLabelNum);
                        ASM_MathI(program,"addi", T1, X0, 1);
                        ASM_Label(program,program->currentLabelNum);
                        ASM_MathI(program,"xori", T1, T1, 1);
                        program->currentLabelNum++;
                    }
                    if (strcmp(currentToken->value, "u~") == 0)
                    {
                        debugPrintf("unary sub operator");
                        ASM_Imm(program,T2, (uint32_t)(-1));
                        ASM_Math(program,"xor", T1, T2, T1);
                    }
                    if (strcmp(currentToken->value, "u&") == 0)
                    {
                        if (param1StackLoc == NULL)
                        {
                            SetError(error, POINTERS, ASM_GENERATION, "'&' can only be used on variables", lineNumber);
                            goto exit;
                        }
                        debugPrintf("Unary reference\n");
                        // Pop the parameter load, because it is not needed for this operation
                        PopTargetASMList(program);
                        ASM_Imm(program,T1, param1StackLoc->offset);
                        ASM_Math(program,"add", T0, T1, T1);
                    }
                }

                for (int i = 0; i < operator->parameters + 1; i++)
                {
                    free(DeleteListElement(postfix, index - operator->parameters));
                }
                ASM_StackPush(program,T1);
                UpdateStackList(program,"TEMP", STACK_POINTER);

                Token *newToken = malloc(sizeof(Token));
                newToken->type = STACK_POINTER_TOKEN;
                newToken->lineNumber = currentToken->lineNumber;

                uint16_t stackIndex = ((Scope *)PeekList(&program->scopes))->symbols.count - 1;
                newToken->value = malloc(IntLength(stackIndex) + 1);
                sprintf(newToken->value, "%hu", stackIndex);
                debugPrintf("insert to : %d\n", index - operator->parameters);
                ListInsert(postfix, index - operator->parameters, newToken);
                debugPrintf("set index to 0\n");
                index = 0;
                continue;
            }
            debugPrintf("loopDone\n");
            index++;
        }
    debugPrintf("Postfix converted, tokens left: %d\n", postfix->count);
    delay(100);
    if (postfix->count > 1)
    {
        SetError(error, SYNTAX_ERROR, ASM_GENERATION, "Equation did not simplfiy", lineNumber);
        goto exit;
    }
    Token *token = (Token *)PeekList(postfix);
    debugPrintf("Type: %d\n", token->type);
    debugPrintf("Line: %d\n", token->lineNumber);

    return;
exit:
    return;
}
#endif