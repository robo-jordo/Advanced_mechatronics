#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"i2c_setup.h"
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

void LCD_buttons(unsigned short color){
    int i = 0;
    int k = 0;
    for(i=0; i<40; i++){
        for(k=0; k<40; k++){
            LCD_drawPixel(100+i,60+k,color);
        }
    }
    char string3[20];
    sprintf(string3, "UP");
    LCD_drawString(116, 80, string3 , ILI9341_RED, ILI9341_BLACK);
    
    for(i=0; i<40; i++){
        for(k=0; k<40; k++){
            LCD_drawPixel(100+i,200+k,color);
        }
    }
    sprintf(string3, "DOWN");
    LCD_drawString(108, 220, string3 , ILI9341_RED, ILI9341_BLACK);
}

void IMU_bars(short fillx, short filly, short full_length, unsigned short xcolor,unsigned short ycolor, unsigned short backColor){
    int i = 0;
    int k = 0;
    for(i=-full_length; i<full_length; i++){
        for(k=0; k<8; k++){
            if (fillx<0){
                if(i>=fillx && i<-4){
                    LCD_drawPixel(120-i,156+k,xcolor);
                }
                else{
                    LCD_drawPixel(120-i,156+k,backColor);
                }
            }
            else{
                if(i<=fillx && i>4){
                    LCD_drawPixel(120-i,156+k,xcolor);
                }
                else{
                    LCD_drawPixel(120-i,156+k,backColor);
                }
            }
            if (filly<0){
                if(i>=filly && i<-4){
                    LCD_drawPixel(116+k,160-i,ycolor);
                }
                else{
                    LCD_drawPixel(116+k,160-i,backColor);
                }
            }
            else{
                if(i<=filly && i>4){
                    LCD_drawPixel(116+k,160-i,ycolor);
                }
                else{
                    LCD_drawPixel(116+k,160-i,backColor);
                }
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
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    __builtin_enable_interrupts();
    
    initIMU();
    SPI1_init();
    LCD_init();
    LCD_clearScreen(ILI9341_WHITE);
    LCD_buttons(ILI9341_BLACK);
    char string3[20];
    char string4[20];
    sprintf(string3, "Buttons");
    LCD_drawChar(28, 32, 'M', ILI9341_BLACK, ILI9341_GREEN);
    LCD_drawString(28, 32, string3 , ILI9341_BLACK, ILI9341_GREEN);
    char r ;
    unsigned char data[14];
    short temp = 0;
    short accel_x = 0;
    short accel_y = 0;
    short accel_z = 0;
    short gyro_x = 0;
    short gyro_y = 0;
    short gyro_z = 0;
    unsigned short x, y; 
    int z; 
    int score= 0;
    int active;
    int state = 0;
    while(1) {
         I2C_read_multiple(0b1101011,0x20, data , 14);
         temp = (data[1]<<8|data[0]);
         accel_x = (data[9]<<8|data[8]);
         accel_y = (data[11]<<8|data[10]);
         accel_z = (data[13]<<8|data[12]);
         gyro_x = (data[3]<<8|data[2]);
         gyro_y = (data[5]<<8|data[4]);
         gyro_z = (data[7]<<8|data[6]);
        _CP0_SET_COUNT(0);

        while(_CP0_GET_COUNT()<1190000){
            ;
        }
        
        XPT2046_read(&x, &y, &z);
        sprintf(string4, "         ");
        LCD_drawString(160, 32, string4  , ILI9341_BLACK, ILI9341_WHITE);
        LCD_drawString(160, 52, string4  , ILI9341_BLACK, ILI9341_WHITE);
        sprintf(string4, "X: %d" , x );
        LCD_drawString(160, 32, string4  , ILI9341_BLACK, ILI9341_GREEN);
        
        sprintf(string4, "Y: %d" , y );
        LCD_drawString(160, 52, string4  , ILI9341_BLACK, ILI9341_GREEN);
        sprintf(string4, "z: %d" , z );
        LCD_drawString(160, 72, string4  , ILI9341_BLACK, ILI9341_GREEN);
        if (z>7500){
            active = 1;
        }
        
        if (active==1){
            if(x<2500 && x>1956){
                if(y<3264 && y>2880){
                    state = +1; 
                }
                else if(y<1700 && y>1300){
                    state = -1;
                }
            }
            if((z<7500)){
                
                active =0;
                score = score + state;
                sprintf(string4, "         ");
                LCD_drawString(100, 170, string4  , ILI9341_BLACK, ILI9341_GREEN);
                sprintf(string4, "score: %d" , score );
                LCD_drawString(100, 170, string4  , ILI9341_BLACK, ILI9341_GREEN);
            }
        }
        else{
            state = 0;
        }
        
        //IMU_bars(accel_x/100, accel_y/100, 100, ILI9341_GREEN,ILI9341_RED , ILI9341_WHITE);
        r = getIMU();
        if (r == 0b01101001){
            LATAbits.LATA4 = !LATAbits.LATA4;
        }
        else{
            LATAbits.LATA4 = 0;
        }
        
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
	// remember the core timer runs at half the sysclk
    }
}
