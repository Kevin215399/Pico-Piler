char LEDBlinkExample[] = {R"(
void main(){
    pinMode(25,OUTPUT);
    while(true){
        pinToggle(25);
        int c = 0;
        while (c< 1000000){
            c += 1;
        }
    }
    return;
}
// This is a simple blink program.
// It quickly toggles pin 25, which is the builtin LED on the Pico 2

// This code, which is in C, gets converted to ASM code.
// This is deceptively complex, because ASM has no builtin variables, functions, or memory management
)"};
char SimonSaysExample[] = {R"(
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
// This is an advanced Simon says program
// Pins 16, 17, 18 -> Red, green, blue LED
// Pins 2, 3, 4, 5 -> Red, green, blue, and yellow buttons
// Pin 6 -> active buzzer

// The program plays a sequence of colors, and the user must repeat it with the buttons
// The sequence is stored in memory using memWrite()

// This code, which is in C, gets converted to ASM code.
// This is deceptively complex, because ASM has no builtin variables, functions, or memory management
)"};
