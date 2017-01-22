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
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Custom Tasks/Servo/ServoQueueParams.h"
#include "decodertask.h"
#include "partest.h"

QueueHandle_t xKPOutputQueue = NULL;
QueueHandle_t xKeypadInputQueue = NULL;

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION_PROTO( vDecoderTask, pvParameters );

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vDecoderTask, pvParamaters )
{
char8 cButton;                      /* the button pressed */
bool bSend = false;                 /* if we are sending a value to the servo task then this is true, else false */
xServoQueueParams_t xToServo;  /* sending stuff to the servos means it needs to be packaged up into a struct */
( void ) pvParamaters;              /* get rid of warnings */
    
for(;;)
    {
        /* do nothing until something is received in the queue */
        if( pdTRUE == xQueueReceive( xKeypadInputQueue, &cButton, portMAX_DELAY ) )
        {
            /* I only have one servo for now */
            xToServo.xServo = BaseRotation;
            
            switch( cButton )
            {
                case 'a':
                    xToServo.xDirection = ADD;
                    bSend = true;
                    break;
                case 'b':
                    xToServo.xDirection = SUB;
                    bSend = true;
                    break;
                default:
                /* all other button pressed don't mean anything with one servo */
                    bSend = false;
                    break;
                
            }
            
            if( true == bSend )
            {
                if( pdFALSE == xQueueSend( xKPOutputQueue, ( void* )&xToServo, ( TickType_t ) portMAX_DELAY ) )
                {
                    /* error sending to the servo queue */
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------*/
QueueHandle_t xStartDecoderTask( int priority, xDecoderParams_t xParams )
{
    /* create the input q and attache the output queue */
    xKeypadInputQueue = xQueueCreate( DECODER_INPUT_QUEUE_SIZE, sizeof(signed char) );
    xTaskCreate( vDecoderTask, "Decoder", configMINIMAL_STACK_SIZE, ( void * ) &xParams, priority, NULL );
    xKPOutputQueue = *(xParams.pxDecoderOutputQueue);
    return xKPOutputQueue;
}

/* [] END OF FILE */
