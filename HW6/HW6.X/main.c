#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include"ili9341.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL =  MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

void LCD_drawChar(int x, int y, char c, unsigned short color, unsigned short backColor){
    int i = 0;
    int k = 0;
    for(i=0; i<5; i++){
        for(k=0; k<8; k++){
            if(ASCII[(c-0x20)][i]&(0b1<<k)){
                LCD_drawPixel(x+i,y+k,color);
            }
            else{
                LCD_drawPixel(x+i,y+k,backColor);
            }
        }
    }
}

void LCD_drawString(int x, int y, char* array, unsigned short color, unsigned short backColor){
    int l = 0;
    while(array[l]!=0){
        LCD_drawChar(x+l*6, y, array[l], color, backColor);
        l++;
    }
}

void LCD_progressBar(int x, int y, int fill, int full_length, unsigned short color, unsigned short backColor){
    int i = 0;
    int k = 0;
    for(i=0; i<full_length; i++){
        for(k=0; k<8; k++){
            if(i<=fill){
                LCD_drawPixel(x+i,y+k,color);
            }
            else{
                LCD_drawPixel(x+i,y+k,backColor);
            }
        }
    }
    
}
    
int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;

    __builtin_enable_interrupts();
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(ILI9341_WHITE);
//    int g =0;
//    int j =0;
//    for(g; g<10; g++){
//        for(j; j<10; j++){
//            LCD_drawPixel(200+j, 300+g, ILI9341_GREEN);
//        }
//    }
    int g = 0;
    char string3[20];
    char string4[20];
    sprintf(string3, "Hello world !");
    LCD_drawChar(28, 32, 'M', ILI9341_BLACK, ILI9341_WHITE);
    LCD_drawString(28, 32, string3 , ILI9341_BLACK, ILI9341_WHITE);
    //LCD_progressBar(100, 200, 50, 100,ILI9341_BLACK , ILI9341_WHITE);
    
    while(1) { 
        if(g<100){
           g++; 
        }
        else{
            g = 0;
        }
        float freq = 1/(_CP0_GET_COUNT()/24000000.0);
        LCD_progressBar(28, 50, g, 100,0x000000+g*2+(g*2<<8)+(g*2<<16) , ILI9341_WHITE);
        sprintf(string4, "%.2f Hz", freq );
        //sprintf(string4, string4);
        LCD_drawString(160, 32, string4  , ILI9341_BLACK, ILI9341_GREEN);
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT()<2400000){
            
            if (PORTBbits.RB4 == 1){
                LATAbits.LATA4 = !LATAbits.LATA4;
            }
            else{
                LATAbits.LATA4 = 0;
            }
        }
        
        
    }
}
