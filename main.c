#include "gpio.h"
#include "uart.h"
#include "framebuffer.h"
#include "monkey.h"

void printf(char *str) {
	uart_puts(str);
}

int main()
{
    init_framebuffer(); // You can use framebuffer, width, height and pitch variables available in framebuffer.h
    
    // Draw a (Green) pixel at coordinates (10,10)
    //drawPixel(10,10,0xFF00FF00);
    int x = 0;
    int y = 300;
    unsigned int color = 0xFF00FF00;
    int fill = 1;

    //void drawRect(x, x, y, y, color, fill);

    fillScreen(0x40e729);

    drawImage(monkey.pixel_data, monkey.width, monkey.height, 0, 100);

    // Print a message to the UART.
    printf("we printed a green pixel on the screen!!\n");
    
    return 0;
}