#include "gpio.h"
#include "uart.h"
#include "framebuffer.h"
#include "monkey.h"
#include "snakeMenu.h"
#include "snakeHeadRight.h"
#include "title.h"
#include "challenge1.h"
#include "challenge2.h"
#include "quit.h"

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

// Setup global variables
unsigned *clo = (unsigned* ) CLO_REG;
int buttons[16];

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


int main()
{
    init_framebuffer(); // You can use framebuffer, width, height and pitch variables available in framebuffer.h
    // Draw a (Green) pixel at coordinates (10,10)
    //drawPixel(10,10,0xFF00FF00);
    fillScreen(0x7FFFFF);
    //void drawRect(x, x, y, y, color, fill);
    // 0,0 to 1023, 767 from the top left corner
    drawImage(title.pixel_data, title.width, title.height, 384, 150);
    drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 168, 350);
    drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 512, 350);
    drawImage(challenge1.pixel_data, challenge1.width, challenge1.height, 240, 350);
    drawImage(challenge2.pixel_data, challenge2.width, challenge2.height, 584, 350);
    Init_GPIO();
    // i regesters button
    // 
    int i = 0;
    // y regesters when a button stops being pressed
    int y = 0;
    while(1){
        i = 0;
        y = 1;
        while(i == 0) i = READ_SNES();
        while(y != 0) y = READ_SNES();
        if (i == 1) {
            drawImage(snakeHeadRight.pixel_data, snakeHeadRight.width, snakeHeadRight.height, 512, 350);
        }
        wait(150000);
    }
    
    return 0;
}
