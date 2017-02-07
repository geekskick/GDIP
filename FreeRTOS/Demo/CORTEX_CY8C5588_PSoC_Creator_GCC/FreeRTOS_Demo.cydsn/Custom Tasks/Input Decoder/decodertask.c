/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 2
Date           : 6th Feb 2017
Changes Made   : 
    Subscription to the mode manager.
    Declarations for the callback created,
    still need to implement init mode stuff.
*****************************************
Change ID      : NA
Version        : 3
Date           : 7th Feb 2017
Changes Made   : 
    Init Mode decoder created
*****************************************/

/* Scheduler include files. */
#include <stdlib.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Custom Tasks/Servo/ServoQueueParams.h"
#include "decodertask.h"
#include "partest.h"
#include "Custom Tasks/Display/globaldisplay.h"
#include "Custom Tasks/Mode Manager/modeManager.h"

QueueHandle_t xDecoderOutputQueue = NULL;
QueueHandle_t xKeypadInputQueue = NULL;
bool ( *prvModeDecoder )( xServoQueueParams_t *pxToServo, char8 cbutton );

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION_PROTO( vDecoderTask, pvParameters );
void prvCreateServoMovementStruct( xServoNumber_t xServo, xServoDirection_t xDirectionToMove, xServoQueueParams_t *pxQueueArgs );
bool prvManualModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvInitModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvTrainingModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvAutoModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
void prvOnModeChange( xMode_t xNewMode );

/*-----------------------------------------------------------------------*/
void prvOnModeChange( xMode_t xNewMode )
{
    /* Init mode just sends button presses the the screen, and does a mode change if the
    relevant button is pressed. The arm is moved to position on startup by the main's servoStart
    function */
    #warning INCOMPLETE MODE CHANGE
    switch( xNewMode )
    {
        case INIT:      prvModeDecoder = &prvInitModeDecoder;   break;
        case MANUAL:    prvModeDecoder = &prvManualModeDecoder; break;
        case TRAINING:  prvModeDecoder = &prvManualModeDecoder; break;
        case AUTO:  prvModeDecoder = &prvManualModeDecoder;     break;
        default: break;
    }
}

/*-----------------------------------------------------------------------*/
bool prvInitModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton )
{
    bool bSend = false; // Don't move a servo at all in init mode
            
    switch( cbutton )
    {
        case 'p':
            vModeChange();
            break;
        default:
            vSendToDisplayQueue( &cbutton, 1 );
        /* all other button pressed don't mean anything servo */

            break;
        
    }

    return bSend;
    
}

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
            // debugging 
            vWriteToComPort( "Rx'd from keypad: ", strlen( "Rx'd from keypad: ") );
            vWriteToComPort( &cButton, 1 );
            vWriteToComPort( "\r\n", 2 );
            
            bSend = prvModeDecoder( &xToServo, cButton );
            
            if( true == bSend )
            {
                if( pdFALSE == xQueueSend( xDecoderOutputQueue, ( void* )&xToServo, ( TickType_t ) portMAX_DELAY ) )
                {
                    /* error sending to the servo queue */

                }
                else
                {
                    // debugging
                    vWriteToComPort( "Sent to servo\r\n", strlen( "Sent to servo\r\n") );
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
    xDecoderOutputQueue = *(pxParams->pxDecoderOutputQueue);

    //needs to be init mode once made
    prvModeDecoder = &prvManualModeDecoder;
    
    vSubscribeToModeChange( &prvOnModeChange );
    
    return xKeypadInputQueue;
}

/*-----------------------------------------------------------------------*/
void prvCreateServoMovementStruct( xServoNumber_t xServo, xServoDirection_t xDirectionToMove, xServoQueueParams_t *pxQueueArgs )
{
    pxQueueArgs->xDirection = xDirectionToMove;
    pxQueueArgs->xServo = xServo;
}

/*-----------------------------------------------------------------------*/
bool prvManualModeDecoder( xServoQueueParams_t *pxToServo, char8 cButton )
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
        case 'p':
            vModeChange();
        default:
        /* all other button pressed don't mean anything servo */
            bSend = false;
            break;
        
    }

    return bSend;
}

/* [] END OF FILE */
