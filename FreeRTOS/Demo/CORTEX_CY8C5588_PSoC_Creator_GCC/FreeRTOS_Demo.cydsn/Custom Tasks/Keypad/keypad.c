/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : 9
Version        : 2
Date           : 3rd Jan 2017
Changes Made   : 
    Changed keypad button detection from 
    checking a button press every 200ms
    to checking every 20ms - as a debounce.
*****************************************
Change ID      : 8
Version        : 3
Date           : 3rd Jan 2017
Changes Made   : 
    Debounce is variable based on tuning pot value
    then sent to the com port for debugging.
*****************************************
Change ID      : NA
Version        : 4
Date           : 10th Feb 2017
Changes Made   : 
    Tuning pot removed as value found 
    through trial and error testing.
    Debug statements removed
*****************************************
Change ID      : NA
Version        : 5
Date           : 12th Feb 2017
Changes Made   : 
    task handle saved in global
*****************************************/

/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdbool.h>

#include "serial.h" // for the serial print
#include "partest.h"

#include "keypad.h"
#include "Custom Tasks/Display/globaldisplay.h"
#include "Custom Tasks/Error/ErrorMode.h"

   /* The keypad output rows one at a time are the same as the columns 
     Keypad layout in rows and columns
    ------------------------
     ROW_0 |  00  01  02  03
     ROW_1 |  10  11  12  13
     ROW_2 |  20  21  22  23
     ROW_3 |  30  31  32  33
    ------------------------
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
#define KEYPAD_ROW_COUNT 4
#define KEYPAD_COL_COUNT KEYPAD_ROW_COUNT

/* incases of error in conversion */
#define KEYPAD_ERROR    0xFF
#define KEYPAD_ERROR_SC '\0' /* signed char version needed */
#define KEYPAD_NO_PRESS 0x00

#define KEYPAD_TASK_PERIODICITYms   10       /* how often the whole keypad needs to be scanned for button press */

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vKeypadTask, pvParameters );
uint8_t prvusGetBitSet( uint8_t usToTest );
signed char prvcButtonToASCII( uint8_t usRow, uint8_t usColumn );
signed char prvcDetectSinglePress( void );

/*-----------------------------------------------------------------------*/
const char *KEYPAD_BUTTONS_ORDERED = "abodefchijglmnkp";    /* these are in the wrong order due to a suspected wiring fault in the keypad 
                                                                where the 3rd columns' rows seems to have been shifted downwards by 1 */

QueueHandle_t xOutputQueue = NULL;                    /* The output values are put into this queue */
TaskHandle_t *pxTaskToNotify = NULL;                  /* the task to notify when a button is pressed */

/* to access the port values by index define then in an array to make it easier to use in code.
also, const and static so that it's stored in flash */
static const uint8_t usLOOKUP_ROW[KEYPAD_ROW_COUNT] = 
{
    KEYPAD_ROW_0, KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3
};

static const uint8_t usLOOKUP_COL[KEYPAD_COL_COUNT] = 
{
    KEYPAD_COL_0,KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3
};

/*-----------------------------------------------------------------------*/
/* This function gets the index of the bit set in the value passed in. If more than one, or none
then return KEYPAD_ERROR value
*/
uint8_t prvusGetBitSet( uint8_t usToTest )
{
uint8_t usReturn;                    /* The return value */
int i;                               /* iterator */
uint8_t usNumBitsSet = 0;            /* count of the number of bits set in the byte */
    
    /* for the number of rows, shift the bits futher right and test if
    the LSB is set, if it is then the value in i is the bit which was set
    eg  0100 >> 0 = 0100     0100 & 1 = 0
        0100 >> 1 = 0010     0010 & 1 = 0
        0100 >> 2 = 0001     0001 & 1 = 1 therefore bit with index of 2 is set
        0100 >> 3 = 1000     1000 & 1 = 0 note the wrap around from lsb to msb which is assumed. This may 
    actually move into the status register but it doesn't matter in this case
    */
    for(i = 0; i < KEYPAD_ROW_COUNT; i++)
    {
        if( ( ( usToTest >> i ) & 0x01 ) == 0x01 )
        {
            usReturn = i;
            usNumBitsSet++;
        }
    }
    
    /* not exactly 1 bit set? then ERROR */
    return usNumBitsSet == 1 ? usReturn : KEYPAD_ERROR;
}

/*-----------------------------------------------------------------------*/
signed char prvcDetectSinglePress( void )
{
uint8_t  usColumnInput = 0;               /* The input value will go here  when read in */
uint8_t  usRow = 0;                       /* The row being energised */
signed char cPressed = KEYPAD_NO_PRESS;   /* The ascii version of the button press needs to be signed char for the vSerialPutString in debugging */
    
    
    /* Iterate over the rows of the keypad, energizing them in turn and polling for the column input */
    for( usRow = 0; usRow < KEYPAD_ROW_COUNT; usRow++ )
    {      
        /* using the pins needs the isr's all disabled so mark it as a critical section of code */
        portENTER_CRITICAL();
        keypadOutPins_Write( usLOOKUP_ROW[usRow] );
        
        usColumnInput = keypadInPins_Read();
        portEXIT_CRITICAL();
        
        /* if there was a button press detected then convert to ASCII and store as the recent press.
        */
        if( usColumnInput != 0 )
        {
            cPressed = prvcButtonToASCII( usRow, usColumnInput ); 
            if( cPressed != KEYPAD_ERROR_SC )
            {
                //vParTestToggleLED(1);
                break; /* no need to do anything else as a button detected */
            }
            else {
                /* an error now means no press */
                cPressed = KEYPAD_NO_PRESS;
            }
        }
    }
    
    return cPressed;
    
}

/*-----------------------------------------------------------------------*/
signed char prvcButtonToASCII( uint8_t usRow, uint8_t usColumn )
{
uint8_t usRowBitSet;                    /* The bit set in the row */
uint8_t uscolBitSet;                    /* The bit set in the col */
uint8_t usIndex;                        /* The index into the KEYPAD_BUTTONS which the row/column combination relates to */
signed char cReturn = KEYPAD_ERROR_SC;  /* the return value is error at init */
    
    usRowBitSet = usRow;
    uscolBitSet = prvusGetBitSet(usColumn);
    
    /* If there are no errors in the bit set detection then use the formula 
        index = 4r+c where r = the row energised and c is the column return. 
    This will give the index into the ascii values to return 
    */
    if( ( usRowBitSet != KEYPAD_ERROR ) && ( uscolBitSet != KEYPAD_ERROR ) )
    {
        /* bitshift by 2 is the same as multiplication by 4 */
        usIndex = ( usRowBitSet << 2 ) + uscolBitSet;
        cReturn = KEYPAD_BUTTONS_ORDERED[usIndex];
        
    }
    return cReturn;
}

/*-----------------------------------------------------------------------*/
/* The main task function energises the keypad row by row and polls the columns
returned. If a column is 1 then that means that at the row/column cominbation a button is pressed.
Send it to the queue as the button's ASCII value
*/
static portTASK_FUNCTION( vKeypadTask, pvParamaters )
{
( void ) pvParamaters;                                          /* stops warnings */
signed char cRecentPressed = KEYPAD_NO_PRESS;                   /* The ascii version of the button press needs to be signed char for the vSerialPutString in debugging */
signed char cPreviousPressed = KEYPAD_NO_PRESS;                 /* the previous button press for debouncing */ 
char msg[DISPLAY_MAX_MSG_LEN];
xDisplayQueueParams xToDisplay;

    /* The whole keypad needs to be checked every KEYPAD_TASK_PERIODICITYms. As there are
       are rows which need checking don't check them all at the same point, space it out 
       evenly so there is time between each row/col check
    */
TickType_t xFrequency = KEYPAD_TASK_PERIODICITYms;    /* The milliseconds between each row check */
TickType_t xLastWakeTime;                             /* For measuring the wait */
    
    /* The variable needs to be initialised before use in the vTaskDelayUntil() function */
    xLastWakeTime = xTaskGetTickCount();
    
    /* the meat of the task */
    for (;;)
    {
        /* provide a debounce for the switch */
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        
        cRecentPressed = prvcDetectSinglePress();
        
        /* If no previous press detected, but a press has been found
           then wait for the debounce period and check the keypad again */
        if( cPreviousPressed == KEYPAD_NO_PRESS && cRecentPressed != KEYPAD_NO_PRESS )
        {
            cPreviousPressed = cRecentPressed;
        }  
        
        /* button has been pressed for more than 2 loops of the task, 
           therefore it counts as a valid, debounced task */
        else if( cRecentPressed == cPreviousPressed && cRecentPressed != KEYPAD_NO_PRESS )
        {
            /* debugging */
            //vParTestToggleLED(0);
            
            /* The qeueue timeout is 0, so if it's full then dont wait, 
            in addition in the vSerialPutString the length is fixed as 
            1 since it's only 1 character for this task. 
            */
            
            if( pdFALSE == xQueueSend( xOutputQueue, ( void* )&cRecentPressed, portMAX_DELAY ) )
            {
                // error in sending to queue 
                vParTestSetLED(1, 0);
                vSetErrorConditon( "Keypad Q Fail \r\n", strlen("Keypad Q Fail \r\n") );
            }
            memset( msg, 0, DISPLAY_MAX_MSG_LEN );
            strcpy( msg, &cRecentPressed );
            vSendToDisplayQueue( msg, strlen(msg), btnPress );
            
            cPreviousPressed = KEYPAD_NO_PRESS;
            
        }
        
        /* No button press has been detected most recently, so clear the previous
           and start checking again
        */
        else
        {
            cPreviousPressed = KEYPAD_NO_PRESS;
        }         
    }   
}

/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t xStartKeypadTask( int priority, xKeypadParams_t *pxParams )
{

    /* init the queue */
    xOutputQueue = *( pxParams->pxOutputQueue );
    xTaskCreate( vKeypadTask, "Keypad", configMINIMAL_STACK_SIZE, NULL, priority, &xKPHandle );
    
    // not input queue
    return NULL;
}

/* [] END OF FILE */
