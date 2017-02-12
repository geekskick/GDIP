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
*****************************************
Change ID      : NA
Version        : 4
Date           : 10th Feb 2017
Changes Made   : 
    Light turns off when queue fails to 
    send.
*****************************************
Change ID      : NA
Version        : 5
Date           : 12th Feb 2017
Changes Made   : 
    All modes decoders in place. Critical
    button presses to prevent extra mode changing
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
#include "Custom Tasks/WPM/WPM.h"
#include "Custom Tasks/Error/ErrorMode.h"

QueueHandle_t xDecoderOutputQueue = NULL;
QueueHandle_t xKeypadInputQueue = NULL;
TaskHandle_t xWPMTask = NULL;
TaskHandle_t *pxKPTask = NULL;
bool ( *prvModeDecoder )( xServoQueueParams_t *pxToServo, char8 cbutton );

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION_PROTO( vDecoderTask, pvParameters );
void prvCreateServoMovementStruct( xServoNumber_t xServo, xServoDirection_t xDirectionToMove, xServoQueueParams_t *pxQueueArgs );
bool prvManualModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvInitModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvTrgModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
bool prvAutoModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton );
void prvOnModeChange( xMode_t xNewMode );
void prvCriticalPress( void );
extern void vModeChange( void );

/*-----------------------------------------------------------------------*/
void prvOnModeChange( xMode_t xNewMode )
{
    /* Init mode just sends button presses the the screen, and does a mode change if the
    relevant button is pressed. The arm is moved to position on startup by the main's servoStart
    function. Context switch is prevented here by the mode manager disabling interrupts */
    switch( xNewMode )
    {
        case INIT:      prvModeDecoder = &prvInitModeDecoder;   break;
        case MANUAL:    prvModeDecoder = &prvManualModeDecoder; break;
        case TRAINING:  prvModeDecoder = &prvTrgModeDecoder;    break;
        case AUTO:      prvModeDecoder = &prvAutoModeDecoder;   break;
        default: break;
    }
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
            
            bSend = prvModeDecoder( &xToServo, cButton );
            
            if( true == bSend )
            {
                if( pdFALSE == xQueueSend( xDecoderOutputQueue, ( void* )&xToServo, ( TickType_t ) 10 ) )
                {
                    /* error sending to the servo queue */
                    vParTestSetLED( 0, 0 );
                    vSetErrorConditon( "Decoder Q Fail \r\n", strlen("Decoder Q Fail \r\n") );

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
    xWPMTask = pxParams->xWPMTaskHandle;
    pxKPTask = pxParams->pxKeypadHandle;
    
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
        case 'm':
            vModeChange();
            prvCriticalPress();

        default:
        /* all other button pressed don't mean anything servo */
            bSend = false;
            break;
        
    }

    return bSend;
}

/*-----------------------------------------------------------------------*/
void prvCriticalPress( void )
{
    if( NULL != pxKPTask )
    {
        vTaskSuspend( *pxKPTask );
        vTaskDelay( ( TickType_t ) 500 ); //half a second delay
        vTaskResume( *pxKPTask );
    }  
}
/*-----------------------------------------------------------------------*/
bool prvTrgModeDecoder( xServoQueueParams_t *pxToServo, char8 cButton )
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
        case 'm':
            vModeChange();
            prvCriticalPress();
            bSend = false;
            break;
        case 'o': // save waypoint o row doesnt work!
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_SAVE, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case 'p': // clear last waypoint should be clear but button doesnt work
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_SAVE, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        default:
        /* all other button pressed don't mean anything servo */
            bSend = false;
            break;
        
    }

    return bSend;
}

/*-----------------------------------------------------------------------*/
bool prvAutoModeDecoder( xServoQueueParams_t *pxToServo, char8 cButton )
{
	bool bSend = true;
            
    switch( cButton )
    {
        case 'm':
            vModeChange();
            prvCriticalPress();
            bSend = false;
            break;
        case 'n': //stop
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_STOP, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case 'l': // reset o row doesnt work
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_RESET, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case 'p': // run
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_RUN, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        default:
        /* all other button pressed don't mean anything servo */
            bSend = false;
            break;
        
    }

    return bSend;
}

/*-----------------------------------------------------------------------*/
bool prvInitModeDecoder( xServoQueueParams_t *pxToServo, char8 cbutton )
{
    bool bSend = false; // Don't move a servo at all in init mode
            
    switch( cbutton )
    {
        case 'm':
            vModeChange();
            prvCriticalPress();
            break;
        default:
            vSendToDisplayQueue( &cbutton, 1 );
        /* all other button pressed don't mean anything servo */

            break;
        
    }

    return bSend;
    
}

/* [] END OF FILE */
