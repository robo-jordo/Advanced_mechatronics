#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>

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
#pragma config WDTPS = PS1048576 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = ON // allow multiple reconfigurations
#pragma config IOL1WAY = ON // allow multiple reconfigurations
#pragma config FUSBIDIO = OFF // USB pins controlled by USB module
#pragma config FVBUSONIO = OFF // USB BUSON controlled by USB module


#include "ili9341.h"
#include "i2c_master_noint.h"
#include "ov7670.h"

#define DIR1 LATAbits.LATA10
#define DIR2 LATAbits.LATA7
#define USER PORTBbits.RB4

int lights[14][3] = {
    {128,0,0},
    {200,0,0},
    {128, 0, 128},
    {200,0,200},
    {0,0,128},
    {0,0,200},
    {0,128,128},
    {0, 200, 200},
    {0,128,0},
    {0, 200, 0},
    {128,128,0},
    {200, 200, 0},
    {255,180,0},
    {150, 100, 0},
};


void startup() {
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // pin definitions
    ANSELA = 0;
    ANSELB = 0;
    TRISAbits.TRISA7 = 0; // DIR2
    DIR2 = 0;
    TRISAbits.TRISA10 = 0; // DIR1
    DIR1 = 0;
    TRISBbits.TRISB4 = 1; // USER
    TRISBbits.TRISB3 = 0; // USER
    TRISBbits.TRISB2 = 0; // USER
    
    LATBbits.LATB3 = 0;
    LATBbits.LATB2 = 0;
    
    // OC1 is B15, goes with DIR1
    RPB15Rbits.RPB15R = 0b0101;
    
    // OC4 is A4, goes with DIR2
    RPA4Rbits.RPA4R = 0b0101;

    
//    // use Timer 3 for PWM
    T3CONbits.TCKPS = 0; // Timer prescaler N=1 (1:1)
    PR3 = 2399; // PR = PBCLK / N / desiredF - 1
    TMR3 = 0; // initial TMR count is 0
    OC1CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
    OC4CONbits.OCM = 0b110;
    OC1CONbits.OCTSEL = 1; // User Timer3
    OC4CONbits.OCTSEL = 1;
    OC1RS = 0; // duty cycle
    OC1R = 0; // initialize before turning OC1 on; afterward it is read-only
    OC4RS = 0;
    OC4R = 0;
    T3CONbits.ON = 1; // turn on Timer
    OC1CONbits.ON = 1; // turn on OC1
    OC4CONbits.ON = 1;
//    
    // LCD uses SPI1: A0 is SDO, A1 is SDI, B5 is CST, B14 is SCK1, A9 is DC, B7 is CS
    SPI1_init();
    LCD_init();
    LCD_clearScreen(ILI9341_DARKGREEN);
    
    // Camera uses C0-7, C8 as OC2, A8 as INT3/PCLK, B13 as INT2/HREF, C9 as INT1/VSYNC, and I2C1
    i2c_master_setup();
    ov7670_setup();
    
    // B3 is available as SCL2, B2 is available as SDA2
}

void tim2Init(void){
//    T2CONbits.TCKPS = 0; // Timer2 prescaler N=1 (1:1)
//    PR2 = 2399; // PR = PBCLK / N / desiredF - 1
//    TMR2 = 0; // initial TMR2 count is 0
//    OC1CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
//    OC1RS = 1200; // duty cycle
//    OC1R = 0; // initialize before turning OC1 on; afterward it is read-only
//
//    
//    OC4CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
//    OC4RS = 2400; // duty cycle
//    OC4R = 0; // initialize before turning OC1 on; afterward it is read-only
//
//    
//    T2CONbits.ON = 1; // turn on Timer2
//    OC1CONbits.ON = 1; // turn on OC1
//    OC4CONbits.ON = 1; // turn on OC4
    
}

void setPWM(int motor, int duty){
    
    if (motor == 1){
        OC1RS = duty; // duty cycle

    }
    else{
        OC4RS = duty; // duty cycle
    }

}

void set_LED(unsigned char r,unsigned char g, unsigned char b){
    int j = 0;
    long colour = (g<<16)|(r<<8)|b;
    for(j=0;j<24;j++) {
        //if(((colour)&&(0b1<<i))==0b0){
        if((colour&(0b1<<(23-j)))==0){
            LATBbits.LATB3 = 1;
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT()<2){
                ;
            }
            _CP0_SET_COUNT(0);
            LATBbits.LATB3 = 0;
            while(_CP0_GET_COUNT()<4){
                ;
            }
            _CP0_SET_COUNT(0);
        }
        else{
            LATBbits.LATB3 = 1;
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT()<4){
                ;
            }
            _CP0_SET_COUNT(0);
            LATBbits.LATB3 = 0;
            while(_CP0_GET_COUNT()<2){
                ;
            }
            _CP0_SET_COUNT(0);
            }
        }
}


int main() {

    __builtin_disable_interrupts();

    startup();
    tim2Init();

    __builtin_enable_interrupts();
    
    int I = 0;
    int light_count = 0;
    char message[100];
    unsigned char d[2000]; // this is the raw camera data, both brightness and color
    unsigned char bright[1000]; // this is for just the brightness data
    int turning = 2;
    int avg_thresh = 80;
    while(1) {

        I++;
//        sprintf(message,"I = %d   ", I);
//        drawString(140,82,message);
//        
        // horizontal read
        /*
        int c = ov7670_count_horz(d);
        sprintf(message, "c = %d   ",c);
        drawString(140,92,message); // there are 290 rows
        
        int x = 0, x2 = 1;
        int y = 0;
        int dim = 0;
        for(x = 0; x < c/2; x++, x2=x2+2){
            dim = d[x2]>>3;
            bright[x] = d[x2];
            for(y=0;y<32;y++){
                if (y == dim){
                    LCD_drawPixel(y+30,x,ILI9341_BLACK);
                }
                else {
                    LCD_drawPixel(y+30,x,ILI9341_WHITE);
                }
            }
        }
        */

        // vertical read
        if (I%10==0){
            LATBbits.LATB2 = 1;
            
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT()<1750000){
            ;
        }
        LATBbits.LATB2 = 0;
        }
        
        _CP0_SET_COUNT(0);
        set_LED(lights[light_count][0],lights[light_count][1],lights[light_count][2]);
        set_LED(lights[light_count][0],lights[light_count][1],lights[light_count][2]);
        set_LED(lights[light_count][0],lights[light_count][1],lights[light_count][2]);
        set_LED(lights[light_count][0],lights[light_count][1],lights[light_count][2]);
                
        if (light_count<13){
            light_count ++;
        }
        else{
            light_count = 0;
        }
        
        while(_CP0_GET_COUNT()<1750000){
            ;
        }
        setPWM(1, 0);
        setPWM(2, 0);
        int c = ov7670_count_vert(d);
        
//        sprintf(message, "c = %d   ",c);
//
//        drawString(140,92,message);
//        
        int x = 0, x2 = 0;
        int y = 0;
        int dim = 0;
        for(x = 0; x < c/2; x++, x2=x2+2){
            dim = d[x2]>>3;
            bright[x] = d[x2];
            for(y=0;y<32;y++){
                if (y == dim){
                    //LCD_drawPixel(x,y+30,ILI9341_BLACK);
                }
                else {
                    //LCD_drawPixel(x,y+30,ILI9341_WHITE);
                }
            }
        }
        
        // at this point, bright has c/2 values in it, each value is a brightness of a pixel
        // loop through bright and calculate where the middle of the dip or bump is
        // then set the motor speed and direction to follow the line
        int i = 0;
        int sum = 0;
        int sumR = 0;
        int com = 0;
        int avg = 0;

        float percentage = 1;
        int largest = 0;
        // find the average brightness
        for (i=0;i<c/2;i++){
            sum = sum + bright[i];
        }
        avg = sum / c/2;

        // threshold and center of mass calculation
        sum = 0;
        largest = 0;
        for (i=0;i<c/2;i++){
            if (bright[i]<avg){
                // count this pixel
   
                //LCD_drawPixel(i,30,ILI9341_BLUE); // visualize which pixels we're counting
                sum = sum + 255;
                sumR = sumR + 255*i;
            }
            else {
                //LCD_drawPixel(i,30,ILI9341_WHITE);
                // don't count this pixel
            }
            if (bright[i]>largest){
                largest = bright[i];
            }
        }
//        if (USER==0){
//            avg_thresh = avg - 5;
//        }
//        sprintf(message, "avg = %d   ",avg);
//        drawString(140,112,message);

        // only use com if the camera saw some data
        if (sum>10){
            com = sumR/sum;
        }
        else {
            com = c/2/2;
        }
        // draw the center of mass as a bar
        for(y=0;y<32;y++){
            //LCD_drawPixel(com,y+30,ILI9341_RED);
        }
        int speed = 0;
        int e = 0;
        int delta_speed = 0;

//        // try to keep com at c/2/2 using the motors
//        DIR1 = 1; // depending on your motor directions these might be different
//        DIR2 = 1;
        // if com is less than c/2/2, then slow down the left motor, full speed right motor
        // if com is more than c/2/2, then slow down right motor, full speed left motor
        // things to play with: the slope of the line, the value that determines when the motor is not full speed
//        sprintf(message, "largest = %d   ",com);
//        drawString(140,162,message);
        if (avg<70){
            if (com < c/2/2-45){
                turning  = 1;
                e = (c/2/2 - com);
                delta_speed  = (int)((e/100)*(250));
                speed = 1500+delta_speed;
                //speed = (2399 - (2399/c/2/2)*e); // when the com is all the way over, the motor is all off
                if(speed > 2399){
                    speed = 2399;
                }
                if(speed < 0){
                    speed = 0;
                }
//                sprintf(message, "motor 1 = %d   ",(int)(2399/percentage));
//                drawString(140,122,message);
//
//                sprintf(message, "motor 2 = %d   ",(int)(speed/percentage));
//                drawString(140,132,message);
                DIR1 = 1; // depending on your motor directions these might be different
                DIR2 = 0;
//                setPWM(1, (int) 2399/percentage);
//                setPWM(2, (int) (speed)/percentage);
                setPWM(1, (int)((speed)*(1.1)));
                setPWM(2, (int)(speed));
                

            }
            else if(com > c/2/2+45){
                turning = 0;
                e = (com - c/2/2);
                delta_speed  = (int)((e/100)*(250));
                speed = 1500+delta_speed;
                //speed = (2399 - (2399/c/2/2)*e); // when the com is all the way over, the motor is all off
                if(speed > 2399){
                    speed = 2399;
                }
                if(speed < 0){
                    speed = 0;
                }
//                sprintf(message, "motor 1 = %d   ",(2399));
//                drawString(140,122,message);
//                sprintf(message, "motor 2 = %d   ",2399);
//                drawString(140,132,message);
                DIR1 = 0; // depending on your motor directions these might be different
                DIR2 = 1;
                setPWM(2, (int)((speed)*(1.1)));
                setPWM(1, (int)(speed));
                
            }
            else{
                DIR1 = 1; // depending on your motor directions these might be different
                DIR2 = 1;
                setPWM(2, 900);
                setPWM(1, 900);
            }
//        sprintf(message, "com = %d   ",com);
//        drawString(140,142,message);
//        sprintf(message, "e = %d   ",e);
//        drawString(140,152,message);
        }
        else{
            if (USER == 0){
                setPWM(2, 1200);
                setPWM(1, 600);
            }
            
            else{
                setPWM(1, 1200);
                setPWM(2, 600);
            }
        }

    }
}
