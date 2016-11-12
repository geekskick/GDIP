/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* Scheduler include files. */
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "serial.h" // for the serial print
#include "partest.h"

#include "keypad.h"

   /* The keypad output rows one at a time are the same as the columns 
     Keypad layout in rows and columns
    ------------------------
     ROW_0 |  00  01  02  03
     ROW_1 |  10  11  12  13
     ROW_2 |  20  21  22  23
     ROW_3 |  30  31  32  33
    -------------------------
     COL_x |  0   1   2   3
    
    each col and row has it's own pin on the connector so the following bitmasks will apply
    */
   
#define KEYPAD_ROW_0 0x1
#define KEYPAD_ROW_1 0x2 
#define KEYPAD_ROW_2 0x4
#define KEYPAD_ROW_3 0x8
    
#define KEYPAD_COL_0 KEYPAD_ROW_0
#define KEYPAD_COL_1 KEYPAD_ROW_1
#define KEYPAD_COL_2 KEYPAD_ROW_2
#define KEYPAD_COL_3 KEYPAD_ROW_3
  
/* it's a 4x4 keypad */
#define  KEYPAD_ROW_COUNT 4
#define  KEYPAD_COL_COUNT KEYPAD_ROW_COUNT

#define KEYPAD_ERROR 0xFF

// this needs to be here else an error comes up in the linker
const char *KEYPAD_BUTTONS = "abcdefghijklmnop";

/* to access the port values by index define */
static const uint8_t usLOOKUP_ROW[KEYPAD_ROW_COUNT] = 
{
    KEYPAD_ROW_0, KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3
};

static const uint8_t usLOOKUP_COL[KEYPAD_COL_COUNT] = 
{
    KEYPAD_COL_0,KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3
};

/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vKeypadTask, pvParameters );

static QueueHandle_t outputQueue = NULL;

static xComPortHandle s;
/*-----------------------------------------------------------------------*/
uint8_t usGetBitSet( uint8_t usToTest )
{
uint8_t usReturn = KEYPAD_ERROR;
    int i;
    
    for(i = 0; i < KEYPAD_ROW_COUNT; i++)
    {
        if( ( ( usToTest >> i ) & 0x01 ) == 0x01 )
        {
            usReturn = i;
            break;
        }
    }
    
    return usReturn;
}

/*-----------------------------------------------------------------------*/
static signed char cButtonToASCII( uint8_t usRow, uint8_t usColumn )
{
uint8_t usRowBitSet;    /* The bit set in the row */
uint8_t uscolBitSet;    /* The bit set in the col */
uint8_t usIndex;        /* The index into the KEYPAD_BUTTONS which the row/column combination relates to */
signed char cReturn = '\0';    /* the return value is NULL at init */
    
    usRowBitSet = usRow;
    uscolBitSet = usGetBitSet(usColumn);
    
    /* If there are no errors in the bit set detection then use the formula 
        index = 4r+c where r = the row energised and c is the column return. 
    This will give the index into the ascii values to return 
    */
    if( ( usRowBitSet != KEYPAD_ERROR ) && ( uscolBitSet != KEYPAD_ERROR ) )
    {
        /* bitshift by 2 is the same as multiplication by 4 */
        usIndex = ( usRowBitSet << 2 ) + uscolBitSet;
        cReturn = KEYPAD_BUTTONS[usIndex];
        
    }
    return cReturn;
}
/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vKeypadTask, pvParamaters )
{

( void ) pvParamaters;                                          /* stops warnings */
uint8_t  usColumnInput = 0;                                     /* The input value will go here  when read in */

    /* The whole keypad needs to be checked every KEYPAD_TASK_PERIODICITYms. As there are
       are rows which need checking don't check them all at the same point, space it out 
       evenly so there is time between each row/col check
    */
const TickType_t xFrequency = KEYPAD_TASK_PERIODICITYms / 4;    /* The milliseconds between each row check */
TickType_t xLastWakeTime;                                       /* For measuring the wait */
    
    /* The variable needs to be initialised before use in the vTaskDelayUntil() function */
    xLastWakeTime = xTaskGetTickCount();
    
    /* the meat of the task */
    for (;;)
    {
        /* As the KEYPAD_ROW_x values are 1000, 0100, etc loop over them, rather than 
            increasing the value each time just perform a bitshift rather than multiply by 2
            as maths can be expensive. This will go through the the seuence 0001 0010 0100 1000
            then repeat
        */
        uint8_t usRow;
        uint8_t tits;
        for( usRow = 0; usRow < KEYPAD_ROW_COUNT; usRow++ )
        {
            usColumnInput = 0;
            /* energise the row */
            tits = usLOOKUP_ROW[usRow];
            
            portENTER_CRITICAL();
            keypadOutPins_Write( tits );
            
            /* get input values */
            usColumnInput = keypadInPins_Read();
            portEXIT_CRITICAL();
            
            /* if there are no bits set on the input then no buttons are pressed in that row */
            if( usColumnInput != 0 )
            {
                vParTestToggleLED(0);
                signed char cButton = cButtonToASCII( usRow, usColumnInput );
                
                // debug printing
                vSerialPutString( s, &cButton, 1);
               
                if( cButton != '\0' )
                {
                    xQueueSend( &outputQueue, ( void* )&cButton, 0 );
                } 
            }
            
            vTaskDelayUntil( &xLastWakeTime, xFrequency );
        }
        
    }
    
}

/*-----------------------------------------------------------------------*/
QueueHandle_t* xStartKeypadTask( int priority, xComPortHandle com )
{
    s = com;
    outputQueue = xQueueCreate( KEYPAD_QUEUE_SIZE, sizeof(uint8_t) );
    // xTaskCreate( vComRxTask, "COMRx", comSTACK_SIZE, ( void* )ipInputQueue , uxPriority, ( TaskHandle_t * ) NULL );
    xTaskCreate( vKeypadTask, "Keypad", configMINIMAL_STACK_SIZE, NULL, priority, ( TaskHandle_t * ) NULL );
    return &outputQueue;
}

/* [] END OF FILE */
