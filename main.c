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
#include "bomb1.h"
#include "bomb2.h"
#include "bomb3.h"
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h>
#include <explosion.h>

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
#define black 0x0
#define cyan 0x7FFFFF

// resolution 1920 × 1080
#define resolutionWidth 1363
#define resolutionHight 767

// Setup global variables
unsigned *clo = (unsigned* ) CLO_REG;
// [id][xCoordinate, yCoordinate, Timer]
int heartBuffer[5][2];
int keyBuffer[3][2];
int bombbuffer[3][2] = {{6,2}, {11,16}, {17,10}}; 
int bombclearing[3][3];

void printf1(char *str) {
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

// Menu snake location definition
#define snakeXLeft resolutionWidth/4 - challenge1.width/2 - snakeHeadRight.width
#define snakeXRight 3 * resolutionWidth/4 - challenge2.width/2 - snakeHeadRight.width
#define snakeXMid resolutionWidth/2 - title.width/2 + title.width/6 - snakeHeadRight.width
#define snakeYMid resolutionHight/2
#define snakeYDown 3 * resolutionHight/4
int mainMenu(){
    fillScreen(cyan);
    drawImage(title.pixel_data, title.width, title.height, resolutionWidth/2 - title.width/2, resolutionHight/4);
    drawImage(challenge1.pixel_data, challenge1.width, challenge1.height, resolutionWidth/4 - challenge1.width/2, resolutionHight/2);
    drawImage(challenge2.pixel_data, challenge2.width, challenge2.height, 3 * resolutionWidth/4 - challenge2.width/2, resolutionHight/2);
    drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, snakeXLeft, snakeYMid);
    drawImage(quit.pixel_data, quit.width, quit.height, resolutionWidth/2 - title.width/2 + title.width/6, 3 * resolutionHight/4);
    int selected = 1;
    while(1){
        int input = READ_INPUT();
        // move right
        if (input == 8) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, snakeXRight, snakeYMid);
            drawRect(snakeXLeft, snakeYMid, snakeXLeft + snakeHeadRight.width, snakeYMid + snakeHeadRight.height, cyan, 1); //left
            drawRect(snakeXMid, snakeYDown, snakeXMid + snakeHeadRight.width, snakeYDown + snakeHeadRight.height, cyan, 1); //down
            selected = 2;
        }
        // move left
        if (input == 7) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, snakeXLeft, snakeYMid);
            drawRect(snakeXMid, snakeYDown, snakeXMid + snakeHeadRight.width, snakeYDown + snakeHeadRight.height, cyan, 1); //down
            drawRect(snakeXRight, snakeYMid, snakeXRight + snakeHeadRight.width, snakeYMid + snakeHeadRight.height, cyan, 1); //right
            selected = 1;
        }
        //move down
        if (input == 6) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, snakeXMid, snakeYDown);
            drawRect(snakeXLeft, snakeYMid, snakeXLeft + snakeHeadRight.width, snakeYMid + snakeHeadRight.height, cyan, 1); //left
            drawRect(snakeXRight, snakeYMid, snakeXRight + snakeHeadRight.width, snakeYMid + snakeHeadRight.height, cyan, 1); //right
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
            if (i==6 && j==2){
                spawnBomb(i ,j, 0, 3);
            }else if (i==11 && j==16){
                spawnBomb(i ,j, 1, 2);
            }else if (i==17 && j==10){
                spawnBomb(i ,j, 2, 1);
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

// should update to add lines back
void clearingSquare(int x, int y){
    drawRect(x*32+1, y*32+1, (x+1)*32-1, (y+1)*32-1, 0x00, 1);
}

void spawnBomb(int i, int j, int n, int t){
    clearingSquare(i,j);
    int r1 = *clo%3;
    int r2 = *clo%3;
    // updating the new bomb location
    bombclearing[n][0] = bombbuffer[n][0] + r1;
    bombclearing[n][1] = bombbuffer[n][1] + r2;
    if (t == 3) {
        drawImage(bomb3.pixel_data, bomb3.width, bomb3.height, (bombclearing[n][0])*32, (bombclearing[n][1])*32);
        bombclearing[n][2] = 3;
    }  
    if (t == 2) {
        drawImage(bomb2.pixel_data, bomb2.width, bomb2.height, (bombclearing[n][0])*32, (bombclearing[n][1])*32);
        bombclearing[n][2] = 2;
    } 
    if (t == 1) {
        drawImage(bomb1.pixel_data, bomb1.width, bomb1.height, (bombclearing[n][0])*32, (bombclearing[n][1])*32);
        bombclearing[n][2] = 1;
    } 
}

void updateBomb(int n) {
    if (bombclearing[n][2] == 3) {
        bombclearing[n][2]--;
        drawImage(bomb2.pixel_data, bomb2.width, bomb2.height, (bombclearing[n][0])*32, (bombclearing[n][1])*32);
        return;
    }
    if (bombclearing[n][2] == 2) {
        bombclearing[n][2]--;
        drawImage(bomb1.pixel_data, bomb1.width, bomb1.height, (bombclearing[n][0])*32, (bombclearing[n][1])*32);
        return;
    }
    if (bombclearing[n][2] == 1) {
        clearingSquare(bombclearing[n][0], bombclearing[n][1]);
        drawImage(explosion.pixel_data, explosion.width, explosion.height, (bombclearing[n][0] - 1)*32 + 1, (bombclearing[n][1] - 1)*32 + 1);
        bombclearing[n][2]--;
        return;
    }
    if (bombclearing[n][2] == 0) {
        //for (int i = bombclearing[n][0] - 1; i < bombclearing[n][0] + 1; i++) {
            //for (int j = bombclearing[n][1] - 1; j < bombclearing[n][1] + 1; i++) {
        clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1] - 1);
        clearingSquare(bombclearing[n][0], bombclearing[n][1] - 1);
        clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1] - 1);
        clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1]);
        clearingSquare(bombclearing[n][0], bombclearing[n][1]);
        clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1]);
        clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1] + 1);
        clearingSquare(bombclearing[n][0], bombclearing[n][1] + 1);
        clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1] + 1);
            //}
        //}
        spawnBomb(bombclearing[n][0], bombclearing[n][1], n, 3);
    }
}

void challengeOne(){

    int snakeX = 1;
    int snakeY = 1; 
    fillScreen(0x0);
    makingGrid();
    int input;
    int num = 0;
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
                    clearingSquare(snakeX, snakeY);
                    snakeX++;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 7){
            if (snakeX>1){
                int i = snakeX-1;
                int j = snakeY;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeX, snakeY);
                    snakeX--;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 6){
            if (snakeY<22){
                int i = snakeX;
                int j = snakeY+1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeX, snakeY);
                    snakeY++;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }else if (input == 5){
            if (snakeY>1 ){
                int i = snakeX;
                int j = snakeY-1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeX, snakeY);
                    snakeY--;
                    drawingSnake(snakeX,snakeY);
                }
            }
        }
        // Need a better soltuion for the spawning!!!!
        updateBomb(0);
        updateBomb(1);
        updateBomb(2);
    
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