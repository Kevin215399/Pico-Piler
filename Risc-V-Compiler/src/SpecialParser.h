#ifndef SPECIAL_PARSER_H
#define SPECIAL_PARSER_H

#include "Common.h"
#include "SemanticAnalyzer.h"
#include "Common/Keywords.h"

Identifier *FindSymbol(GeneralList *variableStack, char *symbol);

typedef struct
{
    Token *token;
    uint8_t precedence;
    uint8_t parameters;
} PrecedenceWrapper;

typedef enum
{
    NO_CAST,
    CAST_TYPE,
    CAST_POINTER,
} Cast;

typedef struct FunctionCall
{
    uint8_t paramatersLeft;
    Identifier *templateFunction;
} FunctionCall;

// Function will destroy input list
void ShuntYard(Error* error, GeneralList *tokens, GeneralList *variableStack, GeneralList *outputStack)
{
    debugPrintf("ShuntYard\n");

    if (tokens == NULL)
    {
        SetError(error, UNEXPECTED_NULL, SPECIAL_PARSER, "Missing Tokens reference", 0);
        goto exit;
    }

    GeneralList operatorStack;
    InitializeList(&operatorStack);

    // Special handling for type casts
    Cast castPhase = NO_CAST;

    GeneralList functionCalls;
    InitializeList(&functionCalls);

    Token *lastToken = NULL;
    Token *currentToken = NULL;

    while (tokens->count > 0)
    {
        debugPrintf("loop start, %d left\n", tokens->count);
        debugPrintf("\n");
        delay(100);
        
        lastToken = currentToken;
        currentToken = (Token *)PopListFirst(tokens);
        switch (currentToken->type)
        {
        case SEPERATOR:
        {
            if (castPhase != NO_CAST)
            {
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Cannot do expressions within a cast, use (type pointer)variable", 0);
                goto exit;
            }
            if (currentToken->value[0] == ')')
            {
                // If the operator is parenthesis, pop off all of the previous operations until you get to the matching one
                debugPrintf("parenthesis\n");
                while (
                    operatorStack.count > 0 &&
                    ((PrecedenceWrapper *)PeekList(&operatorStack))->token->value[0] != '(')
                {
                    PrecedenceWrapper *poppedOpp = (PrecedenceWrapper *)PopList(&operatorStack);
                    PushList(outputStack, poppedOpp->token);
                    free(poppedOpp);

                    debugPrintf("moved within parenthesis to operand stack\n");
                }
                FunctionCall *lastFunction = PopList(&functionCalls);
                if (lastFunction != NULL)
                {
                    if (lastFunction->paramatersLeft > 0)
                    {
                        lastFunction->paramatersLeft--;
                    }
                    if (lastFunction->paramatersLeft != 0)
                    {
                        SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Not enough parameters for function", 0);
                        goto exit;
                    }
                    free(lastFunction);
                }

                PrecedenceWrapper *poppedOpp = (PrecedenceWrapper *)PopList(&operatorStack);
                free(poppedOpp);
                free(currentToken);
                castPhase = NO_CAST;
            }
            else if (currentToken->value[0] == '(')
            {
                Token *peekNext = (Token *)ListGetIndex(tokens, 0);
                Token *peekLast = (Token *)PeekList(outputStack);
                PrecedenceWrapper *wrapped = (PrecedenceWrapper *)malloc(sizeof(PrecedenceWrapper));
                wrapped->token = currentToken;
                wrapped->precedence = 0;
                PushList(&operatorStack, wrapped);
                if (peekLast == NULL || peekLast->type != IDENTIFIER)
                {
                    PushList(&functionCalls, NULL);
                }
                if (peekNext == NULL)
                {
                    SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Missing closing parenthesis", 0);
                    goto exit;
                }
                else
                {
                    if (peekNext->type == TYPE)
                    {
                        castPhase = CAST_TYPE;
                    }
                }
            }
            else if (currentToken->value[0] == ',')
            {
                if (functionCalls.count == 0)
                {
                    SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Unexpected token \",\"", 0);
                    goto exit;
                }
                else
                {
                    FunctionCall *lastCall = (FunctionCall *)PeekList(&functionCalls);

                    if (lastCall == NULL || lastCall->paramatersLeft <= 1)
                    {
                        SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Unexpected token \",\"", 0);
                        goto exit;
                    }

                    while (
                        operatorStack.count > 0 &&
                        ((PrecedenceWrapper *)PeekList(&operatorStack))->token->value[0] != '(')
                    {
                        PrecedenceWrapper *poppedOpp = (PrecedenceWrapper *)PopList(&operatorStack);
                        PushList(outputStack, poppedOpp->token);
                        free(poppedOpp);

                        debugPrintf("moved within comma to operand stack\n");
                    }
                    free(currentToken);

                    lastCall->paramatersLeft--;
                }
            }
        }
        break;
        case OPERATOR:
        {
            if (castPhase == CAST_POINTER && currentToken->value[0] == '*')
            {
                PushList(outputStack, currentToken);
                break;
            }
            if (castPhase != NO_CAST)
            {
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Cannot do math within a cast, use (type pointer)variable", 0);
                goto exit;
            }
            debugPrintf("operator token: %s\n", currentToken->value);

            PrecedenceWrapper *wrapped = (PrecedenceWrapper *)malloc(sizeof(PrecedenceWrapper));
            wrapped->token = currentToken;

            uint8_t parameters = 2;
            if (lastToken == NULL || lastToken->type == OPERATOR || strcmp(lastToken->value,"(") ==0)
            {
                // Two operators next to each other, means this one is unary
                debugPrintf("Is unary\n");
                parameters = 1;
            }
            wrapped->parameters = parameters;

            if (parameters == 1)
            {
                char *temp = malloc(strlen(currentToken->value) + 2);
                sprintf(temp, "u%s", currentToken->value);
                free(currentToken->value);
                currentToken->value = temp;
            }

            bool foundOperator = false;
            for (int i = 0; i < OPERATOR_PRECEDENT_COUNT; i++)
            {
                if (OPERATOR_PRECEDENT_LIST[i].parameters == parameters && strcmp(OPERATOR_PRECEDENT_LIST[i].operator, currentToken->value) == 0)
                {
                    debugPrintf("Operator found, index: %d\n", i);
                    wrapped->precedence = OPERATOR_PRECEDENT_LIST[i].precedence;
                    foundOperator = true;
                    break;
                }
            }
            if (!foundOperator)
            {
                debugPrintf("not recognized: %s\n", currentToken->value);
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Operator not recognized", 0);
                goto exit;
            }
            PrecedenceWrapper *peek = ((PrecedenceWrapper *)PeekList(&operatorStack));
            while (operatorStack.count > 0 && peek->precedence >= wrapped->precedence && peek->parameters != 1)
            {
                PrecedenceWrapper *poppedOpp = (PrecedenceWrapper *)PopList(&operatorStack);
                debugPrintf("pop opp\n");
                PushList(outputStack, poppedOpp->token);
                free(poppedOpp);
                peek = ((PrecedenceWrapper *)PeekList(&operatorStack));
            }
            PushList(&operatorStack, wrapped);
        }
        break;
        case LITERAL:
        {
            if (castPhase != NO_CAST)
            {
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Literals not allowed within a cast", 0);
                goto exit;
            }
            debugPrintf("literal token\n");
            PushList(outputStack, currentToken);
        }
        break;
        case IDENTIFIER:
        {
            if (castPhase != NO_CAST)
            {
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Symbols are not allowed within a cast", 0);
                goto exit;
            }
            Identifier *symbol = FindSymbol(variableStack, currentToken->value);
            if (symbol == NULL)
            {
                SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Symbol not defined in current scope", 0);
                goto exit;
            }

            Token *peekNext = (Token *)ListGetIndex(tokens, 0);
            if (peekNext == NULL)
            {
                debugPrintf("no next\n");
                if (symbol->funcOrVar == FUNCTION)
                {
                    SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Symbol is function, but is missing call", 0);
                    goto exit;
                }
                debugPrintf("Pushing identifier out\n");
                PushList(outputStack, currentToken);
                continue;
            }
            if (symbol->funcOrVar == FUNCTION)
            {
                debugPrintf("Found function\n");
                delay(100);
                if (peekNext->value[0] != '(')
                {
                    SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Symbol is function, but is missing call", 0);
                    goto exit;
                }
                FunctionCall *call = malloc(sizeof(FunctionCall));
                call->paramatersLeft = symbol->parameters.count;
                call->templateFunction = symbol;
                PushList(&functionCalls, call);

                PrecedenceWrapper *wrapped = (PrecedenceWrapper *)malloc(sizeof(PrecedenceWrapper));
                wrapped->token = currentToken;
                wrapped->parameters = symbol->parameters.count;
                wrapped->precedence = 255;
                PushList(&operatorStack, wrapped);
            }
            else
            {
                if (peekNext->value[0] == '(')
                {
                    SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Symbol is variable, but has a call", 0);
                    goto exit;
                }
                debugPrintf("Pushing identifier out\n");
                PushList(outputStack, currentToken);
            }
        }
        break;
        case TYPE:
            if (castPhase == CAST_TYPE)
            {
                PushList(outputStack, currentToken);
            }
            SetError(error, SYNTAX_ERROR, SPECIAL_PARSER, "Types can only be used within casts", 0);
            goto exit;

            break;

        default:
        {
            debugPrintf("type none?\n");
        }
        break;
        }
    }
    while (operatorStack.count > 0)
    {
        debugPrintf("pop opp\n");
        PrecedenceWrapper *poppedOpp = (PrecedenceWrapper *)PopList(&operatorStack);
        PushList(outputStack, poppedOpp->token);
        free(poppedOpp);
    }

exit:
    debugPrintf("shunt yard done");
    delay(100);
    return;
}

#endif