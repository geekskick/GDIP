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

#define prvBASE_ROT_P   'd'
#define prvBASE_ROT_M   'c'
#define prvBASE_ELE_P   'h'
#define prvBASE_ELE_M   'g'
#define prvELBOW_P      'l'
#define prvELBOW_M      'k'
#define prvWRIST_ROLL_P 'b'
#define prvWRIST_ROLL_M 'a'
#define prvWRIST_PTCH_P 'f'
#define prvWRIST_PTCH_M 'e'
#define prvGRAB_OPEN    'j'
#define prvGRAB_CLOSE   'i'
#define prvMODE_CH      'm'
#define prvSAVE         'p'
#define prvRUN          'p'
#define prvCLR          'o'
#define prvSTOP         'n'
#define prvRST          'o'

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
    prvModeDecoder = &prvInitModeDecoder;
    
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
        case prvBASE_ROT_P:
            prvCreateServoMovementStruct( BaseRotation, ADD, pxToServo );
            break;
        case prvBASE_ROT_M:
            prvCreateServoMovementStruct( BaseRotation, SUB, pxToServo );
            break;
        case prvBASE_ELE_P:
            prvCreateServoMovementStruct( BaseElevation, ADD, pxToServo );
            break;
        case prvBASE_ELE_M:
            prvCreateServoMovementStruct( BaseElevation, SUB, pxToServo );
            break;
        case prvELBOW_P:
            prvCreateServoMovementStruct( Elbow, ADD, pxToServo );
            break;
        case prvELBOW_M:
            prvCreateServoMovementStruct( Elbow, SUB, pxToServo );
            break;
        case prvWRIST_ROLL_P:
            prvCreateServoMovementStruct( WristRoll, ADD, pxToServo );
            break;
        case prvWRIST_ROLL_M:
            prvCreateServoMovementStruct( WristRoll, SUB, pxToServo );
            break;
        case prvWRIST_PTCH_P:
            prvCreateServoMovementStruct( WristPitch, ADD, pxToServo );
            break;
        case prvWRIST_PTCH_M:
            prvCreateServoMovementStruct( WristPitch, SUB, pxToServo );
            break;
        case prvGRAB_OPEN:
            prvCreateServoMovementStruct( Grabber, ADD, pxToServo );
            break;
        case prvGRAB_CLOSE:
            prvCreateServoMovementStruct( Grabber, SUB, pxToServo );
            break;
        case prvMODE_CH:
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
        case prvBASE_ROT_P:
            prvCreateServoMovementStruct( BaseRotation, ADD, pxToServo );
            break;
        case prvBASE_ROT_M:
            prvCreateServoMovementStruct( BaseRotation, SUB, pxToServo );
            break;
        case prvBASE_ELE_P:
            prvCreateServoMovementStruct( BaseElevation, ADD, pxToServo );
            break;
        case prvBASE_ELE_M:
            prvCreateServoMovementStruct( BaseElevation, SUB, pxToServo );
            break;
        case prvELBOW_P:
            prvCreateServoMovementStruct( Elbow, ADD, pxToServo );
            break;
        case prvELBOW_M:
            prvCreateServoMovementStruct( Elbow, SUB, pxToServo );
            break;
        case prvWRIST_ROLL_P:
            prvCreateServoMovementStruct( WristRoll, ADD, pxToServo );
            break;
        case prvWRIST_ROLL_M:
            prvCreateServoMovementStruct( WristRoll, SUB, pxToServo );
            break;
        case prvWRIST_PTCH_P:
            prvCreateServoMovementStruct( WristPitch, ADD, pxToServo );
            break;
        case prvWRIST_PTCH_M:
            prvCreateServoMovementStruct( WristPitch, SUB, pxToServo );
            break;
        case prvGRAB_OPEN:
            prvCreateServoMovementStruct( Grabber, ADD, pxToServo );
            break;
        case prvGRAB_CLOSE:
            prvCreateServoMovementStruct( Grabber, SUB, pxToServo );
            break;
        case prvMODE_CH:
            vModeChange();
            prvCriticalPress();
            bSend = false;
            break;
        case prvCLR: 
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_CLEAR, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case prvSAVE: 
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
        case prvMODE_CH:
            vModeChange();
            prvCriticalPress();
            bSend = false;
            break;
        case prvSTOP:
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_STOP, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case prvRST:
            xTaskNotify( xWPMTask, WPM_NOTIFICATION_RESET, eSetValueWithOverwrite );
            prvCriticalPress();
            bSend = false;
            break;
        case prvRUN:
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
        case prvMODE_CH:
            vModeChange();
            prvCriticalPress();
            break;
        default:
            vSendToDisplayQueue( &cbutton, 1, btnAccept );
        /* all other button pressed don't mean anything servo */

            break;
        
    }

    return bSend;
    
}

/* [] END OF FILE */
