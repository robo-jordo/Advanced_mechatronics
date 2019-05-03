/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"

#include"i2c_setup.h"
#include <stdio.h>
#include"ili9341.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
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
    
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    bool appInitialized = true;
    char r ;
    unsigned char data[14];
    short temp = 0;
    short accel_x = 0;
    short accel_y = 0;
    short accel_z = 0;
    short gyro_x = 0;
    short gyro_y = 0;
    short gyro_z = 0;
    char string3[20];
    char string4[20];
    sprintf(string3, "IMU bars");
    LCD_drawChar(28, 32, 'M', ILI9341_BLACK, ILI9341_GREEN);
    LCD_drawString(28, 32, string3 , ILI9341_BLACK, ILI9341_GREEN);
        
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
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
            sprintf(string4, "         ");
            LCD_drawString(160, 32, string4  , ILI9341_BLACK, ILI9341_WHITE);
            LCD_drawString(160, 52, string4  , ILI9341_BLACK, ILI9341_WHITE);
            sprintf(string4, "X: %d" , accel_x );
            LCD_drawString(160, 32, string4  , ILI9341_BLACK, ILI9341_GREEN);
        
            sprintf(string4, "Y: %d" , accel_y );
            LCD_drawString(160, 52, string4  , ILI9341_BLACK, ILI9341_GREEN);
            IMU_bars(accel_x/100, accel_y/100, 100, ILI9341_GREEN,ILI9341_RED , ILI9341_WHITE);
            r = getIMU();
            if (r == 0b01101001){
                LATAbits.LATA4 = !LATAbits.LATA4;
            }
            else{
                LATAbits.LATA4 = 0;
            }
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
