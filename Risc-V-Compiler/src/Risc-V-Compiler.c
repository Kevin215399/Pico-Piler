/*

---- ABOUT -----
Welcome to the Pico-Piler! :)

The pico-piler is a C compiler
This means it takes in C code, and converts it to ASM
ASM code can be run directly on CPUs

Sounds simple right? not really...

ASM has no built in variables, functions, or memory management.
So any C code must go through a complex process for a CPU to understand it
My compiler is designed to output ASM for the Pico 2

For more info, check out this visual demo: https://www.youtube.com/watch?v=wplgGrQUDOs

Whoo that was a lot of info. Thanks for checking it out though!
I spent a lot of time on this :)
- Kevin Hoherz

--- How to run -----

A) On the online simulator
    Click the green play button. A console should open, if not click the bottom right corner.
    Resize the console to better see the prompts

B) Using a Pico 2
    Plug in a Pico 2 in bootsel mode, and drag the UF2 file to the pico drive. 
    Open a console with Pico SDK or an IDE of your choice and follow the prompts

---- STATISTICS ----
5115 lines of code
64+ hours spent
A few headaches

*/

#include "Common.h"
#include "Lexer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"
#include "ASMGenerator.h"
#include "SpecialParser.h"
#include "Examples.h"

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
    ASMProgram *program = CreateProgram();
    debugPrintf("starting sem\n");
    delay(100);

    error = SemanticAnylsis(program, tree);
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

    EmptyList(&program->functionList);

    EmptyList(&program->globalList);

    free(program);

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
    printfAlways("Welcome to Pico-Piler!!\n");

    char *sourceCode = NULL;
    while (true)
    {
        printfAlways("\nPlease select one:\n");
        printfAlways("1 - About (Reccomended)\n");
        printfAlways("2 - Examples\n");
        printfAlways("3 - Input source code\n");

        int c = getchar();
        switch (c)
        {
        case '1':
        {
            FlushInputBuffer();
            printfAlways("\n\nWelcome to the Pico-Piler! :)\n");
            printfAlways("The pico-piler is a C compiler\n");
            printfAlways("This means it takes in C code, and converts it to ASM\n");
            printfAlways("ASM code can be run directly on CPUs\n");
            printfAlways("\n");
            printfAlways("Sounds simple right? not really...\n");
            printfAlways("\n");
            printfAlways("ASM has no built in variables, functions, or memory management.\n");
            printfAlways("So any C code must go through a complex process for a CPU to understand it\n");
            printfAlways("My compiler is designed to output ASM for the Pico 2\n");
            printfAlways("\n");
            printfAlways("For more info, check out this visual demo: https://www.youtube.com/watch?v=wplgGrQUDOs\n");
            printfAlways("\n");

            printfAlways("Whoo that was a lot of info. Thanks for checking it out though!\nI spent a lot of time on this :)\n");
            printfAlways("- Kevin Hoherz\n\n");
            printfAlways("1 - Back (Go check out some examples next!)\n");

            c = getchar();
            FlushInputBuffer();

            break;
        }
        break;
        case '2':
        {
            FlushInputBuffer();
            printfAlways("Select an example:\n");
            printfAlways("1 - LED blink (simplest)\n");
            printfAlways("2 - Simon Says\n");
            printfAlways("3 - Cancel\n");

            c = getchar();
            FlushInputBuffer();
            char *sourceCode = NULL;
            switch (c)
            {
            case '1':
                sourceCode = LEDBlinkExample;
                break;
            case '2':
                sourceCode = SimonSaysExample;
                break;
            default:
                sourceCode = NULL;
                break;
            }
            if (sourceCode == NULL)
            {
                printfAlways("Operation Canceled\n");
                break;
            }
            printfAlways("Source code:\n\n%s", sourceCode);

            printfAlways("\nCompile?\n");
            printfAlways("1 - Confirm (you probably want this)\n");
            printfAlways("2 - Cancel\n");

            c = getchar();
            FlushInputBuffer();
            if (c == '1')
            {
                Compile(sourceCode);
                printfAlways("^ This code was GENERATED by the program from the source code, It's not just printed from a constant\n");
            }
            else
            {
                printfAlways("Canceled\n");
                break;
            }
        }
        break;
        case '3':
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

            FlushInputBuffer();
            c = getchar();
            FlushInputBuffer();
            if (c == '1')
            {
                Compile(sourceCode);
                printfAlways("^ This code was GENERATED by the program from the source code, It's not just printed from a constant\n");
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
