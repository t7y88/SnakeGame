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
#include <explosion2.h>

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
#define white 0xFFFFFF

// resolution 1920 × 1080
#define resolutionWidth 1363
#define resolutionHight 767

// Setup global variables
unsigned *clo = (unsigned* ) CLO_REG;
// [id][xCoordinate, yCoordinate, Timer]
int snakeBuffer[1] = {1,1};
int heartBuffer[2][2] = {{26,13}, {5,16}};
int keyBuffer[3][2] = {{2,8}, {23,2}, {3,20}};
int bombbuffer[3][2] = {{6,2}, {11,16}, {17,10}}; 
int bombclearing[3][3];
int keys = 0;
int hearts = 3;

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

void challengeOne(){

    fillScreen(0x0);
    makingGrid();
    trackScore();
    int input;

    while(1){ 
        
        input = READ_INPUT();
        if (input == 3){
            return;
        }else if (input == 8){
            if (snakeBuffer[0]<30){
                int i = snakeBuffer[0]+1;
                int j = snakeBuffer[1];
                
                // Hard coded the conditions so the snake doesnt go through the walls
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeBuffer[0],snakeBuffer[1]);
                    snakeBuffer[0]++;
                    drawingSnake(snakeBuffer[0],snakeBuffer[1]);
                }
            }
        }else if (input == 7){
            if (snakeBuffer[0]>1){
                int i = snakeBuffer[0]-1;
                int j = snakeBuffer[1];
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeBuffer[0],snakeBuffer[1]);
                    snakeBuffer[0]--;
                    drawingSnake(snakeBuffer[0],snakeBuffer[1]);
                }
            }
        }else if (input == 6){
            if (snakeBuffer[1]<22){
                int i = snakeBuffer[0];
                int j = snakeBuffer[1]+1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeBuffer[0],snakeBuffer[1]);
                    snakeBuffer[1]++;
                    drawingSnake(snakeBuffer[0],snakeBuffer[1]);
                }
            }
        }else if (input == 5){
            if (snakeBuffer[1]>1 ){
                int i = snakeBuffer[0];
                int j = snakeBuffer[1]-1;
                if(!((i==10 && (j<8 && j>0)) || (i==21 && (j<5 && j>0)) || (i==4 && (j<10 && j>6)) || (i==24 && (j<16 && j>11)) || (i==7 && (j<22 && j>14)) || (i==17 && (j<23 && j>15)) || (i==13 && (j<13 && j>4)) ||  ((i>0 && i<7) && j==6) || ((i>0 && i<17) && j==13) || ((i>0 && i<10) && j==18) || ((i>23 && i<29) && j==11) || ((i>20 && i<31) && j==16) || ((i>15 && i<31) && j==7))){
                    clearingSquare(snakeBuffer[0],snakeBuffer[1]);
                    snakeBuffer[1]--;
                    drawingSnake(snakeBuffer[0],snakeBuffer[1]);
                }
            }
        }
        checkKeys();
        checkHearts();
        checkBombs();
        updateBomb(0);
        updateBomb(1);
        updateBomb(2);
    }
}


void drawingSnake(int x, int y){
    drawImage(SnakeHead.pixel_data, SnakeHead.width, SnakeHead.height, x*32+1, y*32+1);
}

// should update to add lines back
void clearingSquare(int x, int y){
    drawRect(x*32, y*32, (x+1)*32, y*32, white, 1);
    drawRect(x*32, y*32, x*32, (y+1)*32, white, 1);
    drawRect(x*32+1, y*32+1, (x+1)*32-1, (y+1)*32-1, black, 1);
}

void trackScore(){
    for (int i= 32; i<40; i++){
        for (int j=0; j<24; j++){
            if (i==34 && j==7){
                if (keys == 1){
                    drawImage(key.pixel_data, key.width, key.height, i*32, j*32);
                }else if (keys == 2){
                    drawImage(key.pixel_data, key.width, key.height, i*32, j*32);
                    drawImage(key.pixel_data, key.width, key.height, (i+1)*32, (j)*32);
                }else if (keys == 3){
                    drawImage(key.pixel_data, key.width, key.height, i*32, j*32);
                    drawImage(key.pixel_data, key.width, key.height, (i+1)*32, (j)*32);
                    drawImage(key.pixel_data, key.width, key.height, (i+2)*32, (j)*32);
                }
            }
            if (i==34 && j==12){
                if (hearts == 1){
                   drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                }
                else if (hearts == 2){
                    drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+1)*32, j*32);
                }else if (hearts == 3){
                    drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+1)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+2)*32, j*32);
                }else if (hearts == 4){
                    drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+1)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+2)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+3)*32, j*32);
                }else{
                    drawImage(heart.pixel_data, heart.width, heart.height, i*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+1)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+2)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+3)*32, j*32);
                    drawImage(heart.pixel_data, heart.width, heart.height, (i+4)*32, j*32);
                }
            }
        }
    }
}

void checkKeys(){
    for (int i=0; i<3; i++){
        if ((snakeBuffer[0] == keyBuffer[i][0]) && (snakeBuffer[1] == keyBuffer[i][1])){
            keys++;
            // sets this key at an unreachable postion
            keyBuffer[i][0] = 0;
            keyBuffer[i][1] = 0;
        }
    }
    trackScore();
}

void checkHearts(){
    for (int i=0; i<2; i++){
        if ((snakeBuffer[0] == heartBuffer[i][0]) && (snakeBuffer[1] == heartBuffer[i][1])){
            hearts++;
            // sets this heart at an unreachable postion
            heartBuffer[i][0] = 0;
            heartBuffer[i][1] = 0;  
        }
    }
    trackScore();
}

void checkBombs(){
    for (int i=0; i<2; i++){
        if ((snakeBuffer[0] == bombbuffer[i][0]) && (snakeBuffer[1] == bombbuffer[i][1])){
            hearts--;
            clearingSquare(snakeBuffer[0],snakeBuffer[1]);
            snakeBuffer[0] = 1;
            snakeBuffer[1] = 1;
            drawingSnake(1,1);
        }
    }
    trackScore();
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
            if ((i==23 && j==2) || (i==3 && j==20) || (i==2 && j==8) ){
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
            // making lines at the vertical end of the grid 
            if(j==23){
                drawRect((i)*32, 767, (i+1)*32, 767, 0xFFFFFF, 0);
            }
        }
    }
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
        drawImage(explosion2.pixel_data, explosion2.width, explosion2.height, (bombclearing[n][0] - 1)*32, (bombclearing[n][1] - 1)*32);
        bombclearing[n][2]--;
        return;
    }
    if (bombclearing[n][2] == 0) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; i++) {
                
                clearingSquare(i, j);
        //clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1] - 1);
        //clearingSquare(bombclearing[n][0], bombclearing[n][1] - 1);
        //clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1] - 1);
        //clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1]);
        //clearingSquare(bombclearing[n][0], bombclearing[n][1]);
        //clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1]);
        //clearingSquare(bombclearing[n][0] - 1, bombclearing[n][1] + 1);
        //clearingSquare(bombclearing[n][0], bombclearing[n][1] + 1);
        //clearingSquare(bombclearing[n][0] + 1, bombclearing[n][1] + 1);
            }
        }
        spawnBomb(bombclearing[n][0], bombclearing[n][1], n, 3);
    }
}


abuffer[4][2] = {{0,20}, {0,21},{0,22}, {0,23}};

void challengeTwo(){
    
    fillScreen(0x0);
    makingGrid2();
    int input;

    while(1){ 
        
        input = READ_INPUT();
        if (input == 3){
            return;
        }else if (input==8){
            if (abuffer[0][0]<39){
                int j = abuffer[0][1];
                int i = abuffer[0][0]+1;
                clearingSand(abuffer[3][0], abuffer[3][1]);
                abuffer[3][1] = abuffer[2][1];
                abuffer[3][0] = abuffer[2][0];
                abuffer[2][1] = abuffer[1][1];
                abuffer[2][0] = abuffer[1][0];
                abuffer[1][1] = abuffer[0][1];
                abuffer[1][0] = abuffer[0][0];
                abuffer[0][1] = j;
                abuffer[0][0] = i;
                drawingAnaconda(i,j);
            }
        }else if (input==7){
            if (abuffer[0][0]>0){
                int j = abuffer[0][1];
                int i = abuffer[0][0]-1;
                clearingSand(abuffer[3][0], abuffer[3][1]);
                abuffer[3][1] = abuffer[2][1];
                abuffer[3][0] = abuffer[2][0];
                abuffer[2][1] = abuffer[1][1];
                abuffer[2][0] = abuffer[1][0];
                abuffer[1][1] = abuffer[0][1];
                abuffer[1][0] = abuffer[0][0];
                abuffer[0][1] = j;
                abuffer[0][0] = i;
                drawingAnaconda(i,j);
            }
        }else if (input==6){
            if (abuffer[0][1]<23){
                int j = abuffer[0][1]+1;
                int i = abuffer[0][0];
                clearingSand(abuffer[3][0], abuffer[3][1]);
                abuffer[3][1] = abuffer[2][1];
                abuffer[3][0] = abuffer[2][0];
                abuffer[2][1] = abuffer[1][1];
                abuffer[2][0] = abuffer[1][0];
                abuffer[1][1] = abuffer[0][1];
                abuffer[1][0] = abuffer[0][0];
                abuffer[0][1] = j;
                abuffer[0][0] = i;
                drawingAnaconda(i,j);
            }
        }else if (input==5){
            if (abuffer[0][1]>6){
                int j = abuffer[0][1]-1;
                int i = abuffer[0][0];
                clearingSand(abuffer[3][0], abuffer[3][1]);
                abuffer[3][1] = abuffer[2][1];
                abuffer[3][0] = abuffer[2][0];
                abuffer[2][1] = abuffer[1][1];
                abuffer[2][0] = abuffer[1][0];
                abuffer[1][1] = abuffer[0][1];
                abuffer[1][0] = abuffer[0][0];
                abuffer[0][1] = j;
                abuffer[0][0] = i;
                drawingAnaconda(i,j);
            }
        }
    }
}

void makingGrid2(){
    for (int i= 0; i<40; i++){
        for (int j=6; j<24; j++){
            if ((i == abuffer[0][0] && j == abuffer[0][1]) || (i == abuffer[1][0] && j == abuffer[1][1]) || (i == abuffer[2][0] && j == abuffer[2][1]) || (i == abuffer[3][0] && j == abuffer[3][1])){
                drawingAnaconda(i, j);
            }else{
                clearingSand(i,j);
            }
            if ((i==6 && j==20) || (i==12 && j==8) || (i==23 && j==18) || (i==2 && j==8) || (i==37 && j==6)){
                //drawingTrees(i,j);
            }else if (i==33 && j==20){
                //drwaingPond(i,j);
            }
            drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0x0, 0);
        }
    }
}

void clearingSand(int i, int j){
    drawRect(i*32, j*32, (i+1)*32, (j+1)*32, 0xFDFD96, 1);
}

void drawingAnaconda(int i, int j){

    if (i == abuffer[0][0] && j == abuffer[0][1]){
        drawRect((abuffer[0][0])*32, (abuffer[0][1])*32, ((abuffer[0][0])+1)*32, ((abuffer[0][1])+1)*32, 0x0, 1);
        drawRect((abuffer[1][0])*32, (abuffer[1][1])*32, ((abuffer[1][0])+1)*32, ((abuffer[1][1])+1)*32, 0x0, 1);
        drawRect((abuffer[2][0])*32, (abuffer[2][1])*32, ((abuffer[2][0])+1)*32, ((abuffer[2][1])+1)*32, 0x0, 1);
        drawRect((abuffer[3][0])*32, (abuffer[3][1])*32, ((abuffer[3][0])+1)*32, ((abuffer[3][1])+1)*32, 0x0, 1);
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
        if (selected ==2){
            challengeTwo();
        }
        if (selected == 3) {
            fillScreen(0x0);
            break;
        }
    }
    return 0;
}