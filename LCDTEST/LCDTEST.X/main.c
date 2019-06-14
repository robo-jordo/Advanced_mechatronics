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

#define DIR1 LATAbits.LATA10
#define DIR2 LATAbits.LATA7
#define USER PORTBbits.RB4

 int count  = 0;
 static unsigned char red[240];
 unsigned char green[240];
 unsigned char blue[240];

// Timer 3 interrupt handler 
void __ISR(_TIMER_3_VECTOR, IPL5SOFT) Timer3ISR(void) {
    IFS0bits.T3IF = 0;
    count ++;
// how many times has the interrupt occurred?
    if (count > 100){
        count = 0;
        DIR1 = !DIR1;
        DIR2 = !DIR2;
    }
    setPWM(1, count);
    setPWM(2, count);
// set the duty cycle and direction pin

}

// Timer 3 setup for interrupt
void tim3Init(void){
    PR3 = 29999;
    TMR3 = 0;
    T3CONbits.TCKPS = 0b100;
//    T3CONbits.TGATE = 0;
    T3CONbits.TCS = 0;
    T3CONbits.ON = 1;
    IPC3bits.T3IP = 5;
    IPC3bits.T3IS = 0;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

// Timer 2 setup for OC
void tim2Init(void){
    T2CONbits.TCKPS = 0; // Timer2 prescaler N=1 (1:1)
    PR2 = 2399; // PR = PBCLK / N / desiredF - 1
    TMR2 = 0; // initial TMR2 count is 0
    OC1CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
    OC1RS = 1200; // duty cycle
    OC1R = 0; // initialize before turning OC1 on; afterward it is read-only

    
    OC4CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
    OC4RS = 2400; // duty cycle
    OC4R = 0; // initialize before turning OC1 on; afterward it is read-only

    
    T2CONbits.ON = 1; // turn on Timer2
    OC1CONbits.ON = 1; // turn on OC1
    OC4CONbits.ON = 1; // turn on OC4
    
}

void setPWM(int motor, int duty){
    if (motor == 1){
        OC1RS = ((duty*2400)/100); // duty cycle
    }
    else{
        OC4RS = ((duty*2400)/100); // duty cycle
    }
}


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
    DIR2 = 1;
    TRISAbits.TRISA10 = 0; // DIR1
    DIR1 = 1;
    TRISBbits.TRISB4 = 1; // USER
    
    // OC1 is B15, goes with DIR1
    RPB15Rbits.RPB15R = 0b0101;
    // OC4 is A4, goes with DIR2
    RPA4Rbits.RPA4R = 0b0101;
    
    // LCD uses SPI1: A0 is SDO, A1 is SDI, B5 is CST, B14 is SCK1, A9 is DC, B7 is CS
//    SPI1_init();
//    LCD_init();
//    LCD_clearScreen(ILI9341_BLACK);
    
    // Camera uses C0-7, C8 as OC2, A8 as INT3/PCLK, B13 as INT2/HREF, C9 as INT1/VSYNC, and I2C1
    
    // B3 is available as SCL2, B2 is available as SDA2
    
    
    
    
}

void set_char_arr(int color, char *array){
    int j = 0;
    if (color == 0){
        for (j=0; j<240; j++){
            array[j] = (unsigned char) j;
        }
    }
    if (color == 1){
        for (j=0; j<240; j++){
            green[j] = (unsigned char) 240-j;
        }
    }
    if (color == 2){
        for (j=0; j<240; j++){
            if (j<120){
                blue[j] = (unsigned char) j;
            }
            else{
                blue[j] = (unsigned char) 240-j;
            }
        }
    }
}

void draw_cam(int color, char *array){
    int j = 0;
    int y_base = 0;
    char y_val = 0;
    if (color==0){
        y_base = 0;
        for (j=0; j<240; j++){
            y_val = (array[j]&0b11100000)/5;
            LCD_drawPixel(j, y_base+y_val, ILI9341_RED);
        }
    }
    else if(color ==1){
        y_base = 70;
        for (j=0; j<240; j++){
            y_val = (array[j]&0b11100000)/5;
            LCD_drawPixel(j, y_base+y_val, ILI9341_GREEN);
        }
        
    }
    else{
        y_base = 140;
        for (j=0; j<240; j++){
            y_val = (array[j]&0b11100000)/5;
            LCD_drawPixel(j, y_base+y_val, ILI9341_BLUE);
        }
        
    }
}

int main() {

    __builtin_disable_interrupts();

    startup();
    tim3Init();
    tim2Init();
    set_char_arr(0, red);
    set_char_arr(1, green);
    set_char_arr(2, blue);


    __builtin_enable_interrupts();
    
    int I = 0;
    char message[100];
    setPWM(1,0);
    setPWM(2,0);
       
    while(1) {
        int i =0;
//            draw_cam(0, red);
//            draw_cam(1, green);
//            draw_cam(2, blue);
//        _CP0_SET_COUNT(0);
//        if (USER == 0){
//            setPWM(1,100);
//            setPWM(2,100);
//            DIR1 = 0;
//            DIR2 = 0;
//        }
//        _CP0_SET_COUNT(0);
//        while(_CP0_GET_COUNT()<48000000/2/2){
//            ;
//        }
//        setPWM(1,0);
//        setPWM(2,0);
//        while(_CP0_GET_COUNT()<48000000/2/2){
//            while(USER == 0){}
//        }
//        DIR1 = 0;
//        DIR2 = 1;
//        
          
//        while(_CP0_GET_COUNT()<48000000/2/2){
//            while(USER == 0){}
//        }
//        DIR1 = 1;
//        DIR2 = 0;
//        for(i=0; i<240; i++){
//            sprintf(message,"I = %d   ", (red[i]&0b11100000));
//            drawString(140,92,message);
//        }
        I++;
//        sprintf(message,"I = %d   ", I);
//        drawString(140,92,message);

    }
}