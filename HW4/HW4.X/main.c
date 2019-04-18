#include<xc.h>
#include<math.h>
#include<sys/attribs.h>
//SCK = PB14
//SDO1 = PB5
//SS1 = PB3
//SDI Not used
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


#define CS LATBbits.LATB9
//void init_SPI1();
void setVoltage(char, int);

unsigned char spi_io(unsigned char o){
    SPI1BUF = o;
    while(!SPI1STATbits.SPIRBF){
        ;
    }
    return SPI1BUF;
}

void init_SPI1(){
    //Digital SS pin
    
    //ANSELB = 0;
    TRISBbits.TRISB9 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB1 = 0;
    RPB8Rbits.RPB8R = 0b0011;
    SDI1Rbits.SDI1R = 0b0010;
    
    
    CS = 1;

    //SPI pins
 
    SPI1CON = 0;
    SPI1BUF;
    SPI1BRG = 1;
            
    SPI1STATbits.SPIROV = 0;
    SPI1CONbits.CKE = 1; 
    SPI1CONbits.MSTEN = 1;    
    SPI1CONbits.ON = 1; 
    //SPI1CON=0x8220;              
                
}
//
//void spi_write(unsigned short addr, const char data[], int len){
//    int i = 0;
//    CS = 0;
//    //spi_io(0x2); //might need to change
//    spi_io((addr & 0xFF00)>>8);  //specify write address
//    spi_io(addr & 0x00FF);
//    for (i; i<len; i++){
//        spi_io(data[i]);
//    }
//    CS = 1;
//}

void setVoltage(char channel, int voltage){
    // 0 for chan A
    // 1 for chan B
    unsigned short chan = 0 ;
    if (channel == 0){
        chan = 0b1111000000000000;
    }
    else{
        chan = 0b0111000000000000;
    }
    unsigned short data = 0x0;
    data = data | channel << 15;
    data = data | (0b111 << 12);
    data = data | voltage;
    
    CS = 0;
    spi_io((data & 0xFF00)>>8);
    spi_io(data & 0x00FF);
    CS = 1;
}

int main(void){
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    ANSELB = 0;
    TRISBbits.TRISB3 = 0;
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    TRISBbits.TRISB4 = 1;

    
    init_SPI1();
    __builtin_enable_interrupts();
   
   
//    _CP0_SET_COUNT(0);
//    while(_CP0_GET_COUNT()<11900){
//        ;
//    }
//    _CP0_SET_COUNT(0);
    //setVoltage(0, 500);
    //setVoltage(1, 500);
    //setVoltage(0, 500);
    //setVoltage(1, 500);
    int i = 0;
    int j = 0;
    int top = 0;
    _CP0_SET_COUNT(0);
    while(1) {
        
        float f = 1024 +1024*sin(i*2*3.1415/1000*10);  //should make a 10Hz sin wave)
        float g = j*4;
        i++;
        if (j>98){
            top = 1;
        }
        if (j==0){
            top = 0;
        }
        if (top == 0){
            j++;
        }
        if (top == 1){
            j--;
        }
        
        setVoltage(0, f);
        setVoltage(1, g);
       
        while(_CP0_GET_COUNT()<24000){
            ;
        }
        _CP0_SET_COUNT(0);
        if (PORTBbits.RB4 == 1){
            LATAbits.LATA4 = !LATAbits.LATA4;
        }
        else{
            LATAbits.LATA4 = 0;
        }
    }

    return 0;
}