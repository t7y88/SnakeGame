#include "gpio.h"
#include "uart.h"
#include "framebuffer.h"
#include "snakeHeadRight.h"
#include "title.h"
#include "challenge1.h"
#include "challenge2.h"
#include "quit.h"
#include "heart.h" 
#include "key.h" 
#include "SnakeHead.h"

// Regesters
#define CLO_REG 0xFE003004
#define GPIO_BASE 0xFE200000 
static unsigned *gpio = (unsigned*)GPIO_BASE; // GPIO base

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |= (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

// PINS
#define CLK 11
#define LAT 9
#define DAT 10

// Color
#define cyan 0x7FFFFF

// Setup global variables
unsigned *clo = (unsigned* ) CLO_REG;
int heartBuffer[5] = {1,1,1,1,0};
int keyBuffer[3] = {0,0,0};
int snakeBuffer[2][4] = {{2,2}, {1,2}, {1,1}, {2,1}}; 
// int wallBuffer[2][]
int bomb1buffer[2][6];
int bomb2buffer[2][6];

void printf(char *str) {
	uart_puts(str);
}

// delay from system clock
void wait(int time){
    unsigned c = *clo + time;  
    while(c > *clo);
}

// input/output initlization to pins
void Init_GPIO(){
    INP_GPIO( CLK ); // CLK
    OUT_GPIO( CLK );
    INP_GPIO( LAT ); // LATCH
    OUT_GPIO( LAT );
    INP_GPIO( DAT ); // DATA
}

void Write_Latch(int i){
    if (i == 1) {
        *GPSET0 = 1 << 9;
    } else {
        *GPCLR0 = 1 << 9;
    }
}

void Write_Clock(int i){
    if (i == 1){
        *GPSET0 = 1 << 11;
    } else {
        *GPCLR0 = 1 << 11;
    }
}

int Read_Data(){
    int value = (*GPLEV0 & (1 << 10));
    if (value == 0) {
        return 0;
    } else {
        return 1;
    }
}

// reads inputs snes controller
int READ_SNES(){
    int i = 0;
    int out = 0;
    // sample buttons
    Write_Clock(1);
    Write_Latch(1);
    wait(12);
    Write_Latch(0);
    // pulse loop
    while(i < 16){
        wait(6);
        Write_Clock(0);
        wait(6);
        if (Read_Data() == 0) out = i + 1;
        Write_Clock(1);
        i++;
    }
    return out;
 }

int READ_INPUT(){
    // B Y SL ST UP DOWN LEFT RIGHT A  X  L  R
    // 1 2 3  4  5  6    7    8     9 10 11 12
    int i = 0;
    int y = 1;
    while(i == 0) i = READ_SNES();
    while(y != 0) y = READ_SNES();
    return i;
}

int mainMenu(){
    fillScreen(cyan);
    drawImage(title.pixel_data, title.width, title.height, 384, 150);
    drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 168, 350);
    drawImage(challenge1.pixel_data, challenge1.width, challenge1.height, 240, 350);
    drawImage(challenge2.pixel_data, challenge2.width, challenge2.height, 584, 350);
    drawImage(quit.pixel_data, quit.width, quit.height, 511, 400);
    int selected = 1;
    while(1){
        int input = READ_INPUT();
        if (input == 8) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 512, 350);
            drawRect(168, 350, 168 + snakeHeadRight.width, 350 + snakeHeadRight.height, cyan, 1); //left
            drawRect(439, 400, 439 + snakeHeadRight.width, 400 + snakeHeadRight.height, cyan, 1); //down
            selected = 2;
        }
        if (input == 7) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 168, 350);
            drawRect(439, 400, 439 + snakeHeadRight.width, 400 + snakeHeadRight.height, cyan, 1); //down
            drawRect(512, 350, 512 + snakeHeadRight.width, 350 + snakeHeadRight.height, cyan, 1); //right
            selected = 1;
        }
        if (input == 6) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 439, 400);
            drawRect(168, 350, 168 + snakeHeadRight.width, 350 + snakeHeadRight.height, cyan, 1); //left
            drawRect(512, 350, 512 + snakeHeadRight.width, 350 + snakeHeadRight.height, cyan, 1); //right
            selected = 3;
        }
        if (input == 9) {
            return selected;
        }
    }
    
}

void makingGrid(){
    for (int i= 0; i<32; i++){
        for (int j=0; j<24; j++){
            // making walls on the side 
            if(i==0 || j==0 || i==31 || j==23){
                drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0x964B00, 1);
            }
            //vertical walls
            if((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4))){
                drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0x964B00, 1);
            }
            //horizontal walls
            if(((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7)){
                drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0x964B00, 1);
            }
            //drawing the Snake head
            if(i==1 && j==1){
                drawingSnake(i, j);
            }
            //making hearts
            if ((i==26 && j==13) || (i==5 && j==16) ){
                drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                //**Update Heart Buffer
            }
            // making keys
            if ((i==23 && j==2) || (i==3 && j==20) || (i==2 && j==8)){
                drawImage(key.pixel_data, key.width, key.height, i*32, j*32);
                //**Update Key Buffer
            }
            //highlighting finish
            if ((i==30 && j==22)){
                drawRect(i*32+1, j*32+1, (i+1)*32-1, (j+1)*32-1, 0xFF00, 1);
            }
            // make grid for the rest of the space
            else{
                drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0xFFFFFF, 0);
            }
            // making lines at the end of the grid 
            if (i==31){     
                drawRect(1023, (j)*32, 1023, (j+1)*32, 0xFFFFFF, 0);
            }
            if(j==23){
                drawRect((i)*32, 767, (i+1)*32, 767, 0xFFFFFF, 0);
            }
        }
    }
}

void drawingSnake(int x, int y){
    drawImage(SnakeHead.pixel_data, SnakeHead.width, SnakeHead.height, x*32+1, y*32+1);
}

void clearingSnake(int x, int y){
    drawRect(x*32+1, y*32+1, (x+1)*32-1, (y+1)*32-1, 0x00, 1);
}

void challengeOne(){

    int snakeX = 1;
    int snakeY = 1; 
    fillScreen(0x0);
    makingGrid();
    int input;
    while(1){
        input = READ_INPUT();
        if (input == 3){
            return;
        }else if (input == 8){
            if (snakeX<30){
                int i = snakeX+1;
                int j = snakeY;
                
                // Hard coded the conditions so the snake doesnt go through the walls
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSnake(snakeX, snakeY);
                    snakeX++;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 7){
            if (snakeX>1){
                int i = snakeX-1;
                int j = snakeY;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSnake(snakeX, snakeY);
                    snakeX--;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 6){
            if (snakeY<22){
                int i = snakeX;
                int j = snakeY+1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSnake(snakeX, snakeY);
                    snakeY++;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 5){
            if (snakeY>1 ){
                int i = snakeX;
                int j = snakeY-1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSnake(snakeX, snakeY);
                    snakeY--;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }
    }
}
int main()
{   
    // You can use framebuffer, width, height and pitch variables available in framebuffer.h
    // Draw a (Green) pixel at coordinates (10,10)
    //drawPixel(10,10,0xFF00FF00);
    //void drawRect(x, x, y, y, color, fill);
    // 0,0 to 1023, 767 from the top left corner
    init_framebuffer();

    Init_GPIO();
    int selected;
    while(1){
        selected = mainMenu();
        if (selected == 1){
            challengeOne();
        }
        if (selected == 3) {
            fillScreen(0x0);
            break;
        }
    }
    return 0;
}