#include "Common.h"
#include "Lexer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"
#include "ASMGenerator.h"
#include "SpecialParser.h"

#include <malloc.h>
void PrintMemUsage()
{
    struct mallinfo info = mallinfo();

    debugPrintf("Total allocated: %d bytes\n", info.uordblks);
    debugPrintf("Total free: %d bytes\n", info.fordblks);
    debugPrintf("Total heap size: %d bytes\n", info.arena);
    debugPrintf("Largest free block: %d bytes\n", info.ordblks);
}

void InitializeClasses()
{
    debugPrintf("before cfg\n");
    PrintMemUsage();
    SetupCFG();
    debugPrintf("after cfg\n");
    PrintMemUsage();
}
void FlushInputBuffer()
{

    while (getchar_timeout_us(100) != PICO_ERROR_TIMEOUT)
        ;
}
void Compile(char *sourceCode)
{
    GeneralList tokens;
    InitializeList(&tokens);

    printfAlways("Compiling, this may take some time...\n");

    PrintMemUsage();

    Error *error = LexicalAnylsis(sourceCode, &tokens);
    if (error->errorCode != NO_ERROR)
    {
        PrintError(error);
        free(error->message);
        free(error);
        goto exit;
    }
    free(error);

    printfAlways("Lexer finished...\n");
    PrintMemUsage();

    for (int i = 0; i < tokens.count; i++)
    {
        Token *token = (Token *)ListGetIndex(&tokens, i);
        debugPrintf("Token: %d, %s\n", token->type, token->value);
    }

    TreeNode *tree = (TreeNode *)malloc(sizeof(TreeNode));
    InitializeTree(tree);
    error = AnalyzeSyntax(&tokens, tree);
    if (error->errorCode != NO_ERROR)
    {
        ClearTree(tree);
        free(tree);
        PrintError(error);
        free(error->message);
        free(error);
        goto exit;
    }
    free(error);
    printfAlways("Syntax analyzer finished...\n");
    PrintMemUsage();

    //  goto inspect;

    debugPrintf("starting asm\n");
    delay(100);
    ASMProgram* program = CreateProgram();
    debugPrintf("starting sem\n");
    delay(100);

    error = SemanticAnylsis(program,tree);
    if (error->errorCode != NO_ERROR)
    {
        ClearTree(tree);
        free(tree);
        PrintError(error);
        free(error->message);
        free(error);
        goto exit;
    }
    free(error);
    printfAlways("Semantic Analyzer finished...\n");
    printfAlways("Assembler finished...\n");
    printfAlways("Done!\n");
    PrintMemUsage();

    printfAlways("Assembled code:\n\n");

    PrintASMProgram(program);

    ClearTree(tree);
    free(tree);

    debugPrintf("tree deleted\n");
    delay(100);

    EmptyList(&program->functionList);

    debugPrintf("global deleted\n");
    delay(100);

    EmptyList(&program->globalList);

    debugPrintf("function scope deleted\n");
    delay(100);
    free(program);
    debugPrintf("program deleted\n");
    delay(100);

    PrintMemUsage();
    delay(100);
    debugPrintf("done compile\n");
    delay(100);

exit:
    return;
}
int main()
{
    stdio_init_all();
    sleep_ms(5000);
    debugPrintf("before init\n");
    PrintMemUsage();
    InitializeClasses();
    debugPrintf("after init\n");
    PrintMemUsage();
    printfAlways("Welcome to Pico-Piler\n");

    char *sourceCode = NULL;
    while (true)
    {
        printfAlways("Please select one:\n");
        printfAlways("1 - About\n");
        printfAlways("2 - Input source code\n");
        printfAlways("3 - Examples\n");

        int c = getchar();
        switch (c)
        {
        case '2':
        {
            FlushInputBuffer();
            printfAlways("Input source code:\n");
            GeneralList codeList;
            InitializeList(&codeList);
            bool transmitStarted = false;
            while (true)
            {
                int c = getchar_timeout_us(100);
                if (c == PICO_ERROR_TIMEOUT)
                {
                    if (transmitStarted)
                    {
                        break;
                    }
                    continue;
                }
                transmitStarted = true;

                char *mallocChar = (char *)malloc(1);
                *mallocChar = c;
                PushList(&codeList, mallocChar);
            }
            sourceCode = (char *)malloc(codeList.count + 1);
            uint32_t writeCharIndex = 0;
            while (codeList.count > 0)
            {
                char *character = (char *)PopListFirst(&codeList);
                sourceCode[writeCharIndex++] = *character;
                free(character);
            }
            sourceCode[writeCharIndex++] = '\0';

            printfAlways("\n\nCompile?\n");
            printfAlways("1 - Confirm\n");
            printfAlways("2 - Cancel\n");

            c = getchar();
            if (c == '1')
            {
                FlushInputBuffer();
                Compile(sourceCode);
                printf("\nFinished\n");
                free(sourceCode);
            }
            else
            {
                printfAlways("Canceled\n");
                free(sourceCode);
                sourceCode = NULL;
                break;
            }
        }
        break;
        default:
            printfAlways("Invalid command\n");
            break;
        }
        FlushInputBuffer();
    }

    /*
    sleep_ms(1000);
    char message[32] = {'\0'};
    uint8_t messageIndex = 0;
    GeneralList navigation;
    InitializeList(&navigation);
    while (true)
    {
        char c = (char)getchar();

        if (c == '\n')
        {
            TreeNode *node = TreeGetNode(tree, &navigation);
            if (node == NULL)
                goto getNodeFail;
            uint8_t *value = ((Token *)node->content)->value;

            debugPrintf("Node value: ");
            if (*value >= TERM_OFFSET)
            {
                debugPrintf("%d\n", (int)(*value) - TERM_OFFSET);
            }
            else
            {
                debugPrintf("%s\n", (char *)value);
            }
            int children = (int)node->children.count;
            for (int i = 0; i < children; i++)
            {
                PushByte(&navigation, i);
                TreeNode *node = TreeGetNode(tree, &navigation);
                if (node == NULL)
                    goto getNodeFail;
                uint8_t *value = ((Token *)node->content)->value;
                if (*value >= TERM_OFFSET)
                {
                    debugPrintf("%d, ", (int)(*value) - TERM_OFFSET);
                }
                else
                {
                    debugPrintf("%s, ", (char *)value);
                }
                free(PopList(&navigation));
            }
            debugPrintf("\n");

        getNodeFail:
            EmptyList(&navigation);
            messageIndex = 0;
            memset(message, 0, sizeof(message));
        }
        else if (IsNumber(c))
        {
            PushByte(&navigation, (uint8_t)(c - 48));
            message[messageIndex++] = c;
        }
    }*/

exit:
    while (1)
    {
        sleep_ms(100);
    }
}
