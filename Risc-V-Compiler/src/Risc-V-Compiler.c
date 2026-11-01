#include "Common.h"
#include "Lexer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"
#include "ASMGenerator.h"
#include "SpecialParser.h"
char *sourceCode = R"(

void delay(int c){
    int x = 0;
    while(x<c){
        x+=1;
    }
    return;
}
void NoLight(){
    pinWrite(16,0);
    pinWrite(17,0);
    pinWrite(18,0);
    return;
}
void Light(int l){
    if(l==0){
        pinWrite(16,1);
    } else if(l==1){
        pinWrite(17,1);
    } else if(l==2){
        pinWrite(18,1);
    } else {
        pinWrite(16,1);
        pinWrite(17,1); 
    }
    return;
}
int randomSeed = 67;
int Random(){
    randomSeed ^= randomSeed<<13;
    randomSeed ^= randomSeed>>17;
    randomSeed ^= randomSeed<<5;
    return randomSeed&3;
}
void main()
{
    int arr = malloc(100*4);
    int arrIndex = 0;
    int repeatIndex = 0;
    int gameOver = 0;
    pinMode(16,OUTPUT);
    pinMode(17,OUTPUT);
    pinMode(18,OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);
    pinMode(5,INPUT_PULLUP);
    while(1){
        int next = Random();
        memWrite(arr + 4*arrIndex, next);
        arrIndex += 1;
        
        repeatIndex = 0;
        while(repeatIndex < arrIndex){
            Light(*(arr + 4*repeatIndex));
            delay(4000000);
            NoLight();
            delay(1000000);
            repeatIndex += 1;
        }
        repeatIndex = 0;
        while(repeatIndex < arrIndex){
            int key = 4;
            if(!pinRead(2)){
                key = 0;
            }
            if(!pinRead(3)){
                key = 1;
            }
            if(!pinRead(4)){
                key = 2;
            }
            if(!pinRead(5)){
                key = 3;
            }
            if(key!=4){
                if((*(arr + 4*repeatIndex)) == key){
                    pinWrite(17,HIGH);
                    pinWrite(6,HIGH);
                    delay(1000000);
                    pinWrite(6,LOW);
                    delay(3000000);
                    pinWrite(17,LOW);
                    delay(1000000);
                } else {
                    pinWrite(16,HIGH);
                    pinWrite(6,HIGH);
                    gameOver = 1;
                    break;
                }
                repeatIndex += 1;
            }
        }
        if(gameOver){
            break;
        }
        delay(5000000);
        
    }

    
    delay(5000000);
    pinWrite(6,LOW);
    while(pinRead(2)){
    }
    pinWrite(16,LOW);
    main();

    return 0;
}

)";

void InitializeClasses()
{
    SetupCFG();
}

int main()
{
    stdio_init_all();
    sleep_ms(5000);
    printf("Source:\n\n");
    printf("%s\n", sourceCode);
    sleep_ms(1000);

    InitializeClasses();
    GeneralList tokens;
    InitializeList(&tokens);

    Error *error = LexicalAnylsis(sourceCode, &tokens);
    printf("Done\n");
    if (error->errorCode != NO_ERROR)
    {
        PrintError(error);
        free(error);
        goto inspect;
    }
    free(error);
    for (int i = 0; i < tokens.count; i++)
    {
        Token *token = (Token *)ListGetIndex(&tokens, i);
        printf("Token: %d, %s\n", token->type, token->value);
    }
    sleep_ms(300);

    TreeNode *tree = (TreeNode *)malloc(sizeof(TreeNode));
    InitializeTree(tree);
    error = AnalyzeSyntax(&tokens, tree);
    printf("Done\n");
    if (error->errorCode != NO_ERROR)
    {
        PrintError(error);
        free(error);
        goto inspect;
    }
    free(error);

    //  goto inspect;

    SetASMDestination();

    error = SemanticAnylsis(tree);
    printf("Done\n");
    if (error->errorCode != NO_ERROR)
    {
        PrintError(error);
        free(error);
        goto inspect;
    }
    free(error);

    PrintASMProgram();

inspect:
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

            printf("Node value: ");
            if (*value >= TERM_OFFSET)
            {
                printf("%d\n", (int)(*value) - TERM_OFFSET);
            }
            else
            {
                printf("%s\n", (char *)value);
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
                    printf("%d, ", (int)(*value) - TERM_OFFSET);
                }
                else
                {
                    printf("%s, ", (char *)value);
                }
                free(PopList(&navigation));
            }
            printf("\n");

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
    }
exit:
    while (1)
    {
        sleep_ms(100);
    }
}
