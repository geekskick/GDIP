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
#include "Custom Tasks/Display/globaldisplay.h"

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
#define KEYPAD_ROW_COUNT 4
#define KEYPAD_COL_COUNT KEYPAD_ROW_COUNT

/* incases of error in conversion */
#define KEYPAD_ERROR 0xFF
#define KEYPAD_ERROR_SC '\0' /* signed char version needed */

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vKeypadTask, pvParameters );

/*-----------------------------------------------------------------------*/

const char *KEYPAD_BUTTONS_ORDERED = "abodefchijglmnkp";    /* these are in the wrong order due to a suspected wiring fault in the keypad 
                                                                where the 3rd columns' rows seems to have been shifted downwards by 1 */

QueueHandle_t xOutputQueue = NULL;                    /* The output values are put into this queue */
xComPortHandle xSerialCom;                            /* the output serial comport */
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
uint8_t usGetBitSet( uint8_t usToTest )
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
static signed char cButtonToASCII( uint8_t usRow, uint8_t usColumn )
{
uint8_t usRowBitSet;                    /* The bit set in the row */
uint8_t uscolBitSet;                    /* The bit set in the col */
uint8_t usIndex;                        /* The index into the KEYPAD_BUTTONS which the row/column combination relates to */
signed char cReturn = KEYPAD_ERROR_SC;  /* the return value is error at init */
    
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
TaskHandle_t xDispTask = xGetDisplayTaskHandle();
( void ) pvParamaters;                                          /* stops warnings */
uint8_t  usColumnInput = 0;                                     /* The input value will go here  when read in */
uint8_t  usRow;                                                 /* The row being energised */
signed char cButton;                                            /* The ascii version of the button press needs to be signed char for the vSerialPutString in debugging */

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
        /* Iterate over the rows of the keypad, energizing them in turn and polling for the column input */
        for( usRow = 0; usRow < KEYPAD_ROW_COUNT; usRow++ )
        {      
            /* using the pins needs the isr's all disabled dso mark it as a critical section of code */
            portENTER_CRITICAL();
            keypadOutPins_Write( usLOOKUP_ROW[usRow] );
            
            usColumnInput = keypadInPins_Read();
            portEXIT_CRITICAL();
            
            /* if there are no bits set on the input then no buttons are pressed in that row, 
            if there are then convert it to ascii and write to the output queue, and for debugging, to the 
            serial port too.
            */
            if( usColumnInput != 0 )
            {
                cButton = cButtonToASCII( usRow, usColumnInput );   
               
                /* in case of an error on the ascii conversion and bit shifting make it conditional */
                if( cButton != KEYPAD_ERROR_SC )
                {
                    /* the display task might not be initialied at the very start, 
                    so keep trying to get the taskhandle until a value one is put in the correct location */
                    if( xDispTask == NULL )
                    {
                        xDispTask = *pxTaskToNotify;
                        /* for debugging */
                        vParTestToggleLED(1);
                        
                    }
                    else
                    {
                        /* at this point the xDispTask handle is valid, so send the notifcation, overwriting
                        the current notification. it accepts a 32bit value so cast the ascii to 32 bits in the process.
                        hopefully this does it normally and just prefixes the value with 0's. If not look here for errors
                        in the display not sending out the correct letter
                        */
                        xTaskNotify( xDispTask , ( uint32_t )cButton, eSetValueWithOverwrite );
                    }
                    
                    /* The qeueue timeout is 0, so if it's full then dont wait, 
                    in addition in the vSerialPutString the length is fixed as 
                    1 since it's only 1 character for this task. 
                    */
                    if( pdFALSE == xQueueSend( xOutputQueue, ( void* )&cButton, 0 ) )
                    {
                        /* for debugging */
                        vParTestToggleLED(0);
                    }
              
                } 
            }
            
            vTaskDelayUntil( &xLastWakeTime, xFrequency );
        }
        
    }
    
}

/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t xStartKeypadTask( int priority, xKeypadParams_t *pxParams )
{

    /* init the queue */
    xOutputQueue = *( pxParams->pxOutputQueue );
    xTaskCreate( vKeypadTask, "Keypad", configMINIMAL_STACK_SIZE, NULL, priority, ( TaskHandle_t * ) NULL );
    
    // not input queue
    return NULL;
}

/* [] END OF FILE */
