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

/* the keyad queue is sending an ascii character, so convert it to a number
 * but masking the lower nibble to turn it from a 0x61 = a, into 0x01 etc
 */
#define MAP_IDX(val) ( ( uint8_t )val &0x0F )

QueueHandle_t xDecoderOutputQueue = NULL;
QueueHandle_t xKeypadInputQueue = NULL;

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION_PROTO( vDecoderTask, pvParameters );
void prvCreateServoMovementStruct( xServoNumber_t xServo, xServoDirection_t xDirectionToMove, xServoQueueParams_t *pxQueueArgs );
bool prvManualModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vDecoderTask, pvParamaters )
{
char8 cButton;                      /* the button pressed */
bool bSend = false;                 /* if we are sending a value to the servo task then this is true, else false */
xServoQueueParams_t xToServo;       /* sending stuff to the servos means it needs to be packaged up into a struct */
( void ) pvParamaters;              /* get rid of warnings */
    
for(;;)
    {
        /* do nothing until something is received in the queue */
        if( pdTRUE == xQueueReceive( xKeypadInputQueue, &cButton, portMAX_DELAY ) )
        {
            prvManualModeDecoder( &xToServo, cButton );
            
            if( true == bSend )
            {
                if( pdFALSE == xQueueSend( xDecoderOutputQueue, ( void* )&xToServo, ( TickType_t ) portMAX_DELAY ) )
                {
                    /* error sending to the servo queue */

                }
            }
        }
    }
}

/*-----------------------------------------------------------------------*/
QueueHandle_t xStartDecoderTask( int priority, xDecoderParams_t *pxParams )
{
    /* create the input q and attache the output queue */
    xKeypadInputQueue = xQueueCreate( DECODER_INPUT_QUEUE_SIZE, sizeof(signed char) );
    xTaskCreate( vDecoderTask, "Decoder", configMINIMAL_STACK_SIZE, ( void * ) pxParams, priority, NULL );
    xDecoderOutputQueue = *(xParams->pxDecoderOutputQueue);

    return xKeypadInputQueue;
}

/*-----------------------------------------------------------------------*/
void prvCreateServoMovementStruct( xServoNumber_t xServo, xServoDirection_t xDirectionToMove, xServoQueueParams_t *pxQueueArgs )
{
    pxQueueArgs->xDirection = xDirectionToMove;
    pxQueueArgs->xServo = xServo;
}

/*-----------------------------------------------------------------------*/
bool prvManualModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton )
{
	bool bSend = true;
            
    switch( cButton )
    {
        case 'a':
            prvCreateServoMovementStruct( BaseRotation, ADD, pxToServo );
            break;
        case 'b':
            prvCreateServoMovementStruct( BaseRotation, SUB, pxToServo );
            break;
        case 'c':
            prvCreateServoMovementStruct( BaseElevation, ADD, pxToServo );
            break;
        case 'd':
            prvCreateServoMovementStruct( BaseElevation, SUB, pxToServo );
            break;
        case 'e':
            prvCreateServoMovementStruct( Elbow, ADD, pxToServo );
            break;
        case 'f':
            prvCreateServoMovementStruct( Elbow, SUB, pxToServo );
            break;
        case 'g':
            prvCreateServoMovementStruct( WristRoll, ADD, pxToServo );
            break;
        case 'h':
            prvCreateServoMovementStruct( WristRoll, SUB, pxToServo );
            break;
        case 'i':
            prvCreateServoMovementStruct( WristPitch, ADD, pxToServo );
            break;
        case 'j':
            prvCreateServoMovementStruct( WristPitch, SUB, pxToServo );
            break;
        case 'k':
            prvCreateServoMovementStruct( Grabber, ADD, pxToServo );
            break;
        case 'l':
            prvCreateServoMovementStruct( Grabber, SUB, pxToServo );
            break;
        default:
        /* all other button pressed don't mean anything servo */
            bSend = false;
            break;
        
    }

    return bSend;
}

/* [] END OF FILE */
