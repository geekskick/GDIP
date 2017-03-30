/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 2
Date           : 12th Feb 2017
Changes Made   : 
    mode change switches between input
    queues.
*****************************************/

/* Scheduler include files. */
#include <stdlib.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* my own header */
#include "servo.h"

/* included to toggle the LED */
#include "partest.h"

/* to save the location */
#include "Custom Tasks/Current Position Store/currentposition.h"
#include "servoqueueparams.h"
#include "Custom Tasks/Display/globaldisplay.h"
#include "Custom Tasks/Mode Manager/modeManager.h"

#define SERVO_MAX 8192u    /* servo max value, calculated as per SSD, where the servo also seems to see 2.5 as 2ms */
#define SERVO_MIN 1638u    /* servo min value, calculated as per SSD, where the servo also seems to see 0.5 as 1ms */
#define SERVO_MID SERVO_MIN + ( ( SERVO_MAX-SERVO_MIN ) / 2 )

#define BASE_R_INIT SERVO_MID
#define BASE_E_INIT SERVO_MID + ( SERVO_MID/2 )
#define ELBOW_INIT SERVO_MIN
#define WRIST_R_INIT SERVO_MID
#define WRIST_P_INIT SERVO_MIN
#define GRAB_INIT SERVO_MIN
#define GRAB_MAX SERVO_MID

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );
uint16_t usGetMidPoint( void );
void prvModeChange( xMode_t xNewMode );
void ( *prvGetAndSend )( xArmPosition_t *pxArmPos );
void prvAutoModeRx( xArmPosition_t *pxArmPos );
void prvOtherModeRx( xArmPosition_t *pxArmPos );
uint16_t prvCalculateNewPosition( uint16_t iInitial, uint16_t iOffset, float diff, int i );

/* Each value can either be added to or subtract from */
uint16_t prvAdd ( uint16_t usLHS, uint16_t usRHS );
uint16_t prvSub ( uint16_t usLHS, uint16_t usRHS );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t inputFromDecoderTaskQueue = NULL;
static QueueHandle_t inputFromWPMQueue = NULL;
static uint16_t usServoPeriod = 0;
const static uint16_t usSERVO_SPEED = 4;     /* how far to move the servos on each step, chosen through user experience with the device  */

#define NUM_COEFFS 100
const static float fSMOOTHING_COEFFS[NUM_COEFFS] = { -50.00, -49.98, -49.90, -49.78, -49.61, -49.38, -49.11, -48.80, -48.43, -48.01, -47.55, -47.04, -46.49, -45.89, -45.24, -44.55, -43.82, -43.04, -42.22, -41.35, -40.45, -39.51, -38.53, -37.51, -36.45, -35.36, -34.23, -33.07, -31.87, -30.65, -29.39, -28.10, -26.79, -25.45, -24.09, -22.70, -21.29, -19.86, -18.41, -16.94, -15.45, -13.95, -12.43, -10.91, -9.37, -7.82, -6.27, -4.71, -3.14, -1.57, 0.00, 1.57, 3.14, 4.71, 6.27, 7.82, 9.37, 10.91, 12.43, 13.95, 15.45, 16.94, 18.41, 19.86, 21.29, 22.70, 24.09, 25.45, 26.79, 28.10, 29.39, 30.65, 31.87, 33.07, 34.23, 35.36, 36.45, 37.51, 38.53, 39.51, 40.45, 41.35, 42.22, 43.04, 43.82, 44.55, 45.24, 45.89, 46.49, 47.04, 47.55, 48.01, 48.43, 48.80, 49.11, 49.38, 49.61, 49.78, 49.90, 49.98 };
const static float fMULITPLIER = 31.83098862;
xTaskHandle prvTaskToNotify;
xTaskHandle *pMainTaskAddr = NULL;

/* function pointers to write compare */
void ( *pvWriteCompareFunctions[END] ) ( uint16_t newValue );

/*-----------------------------------------------------------------------*/
void prvModeChange( xMode_t xNewMode )
{
    switch( xNewMode )
    {
        case AUTO:  prvGetAndSend = &prvAutoModeRx; break;
        default:    prvGetAndSend = &prvOtherModeRx; break;
    }
    
}

uint16_t prvCalculateNewPosition( const uint16_t iInitial, const uint16_t iOffset, const float diff, const int i )
{
    return  iInitial - iOffset + ( diff * fSMOOTHING_COEFFS[i] );
}
/*-----------------------------------------------------------------------*/
void prvAutoModeRx( xArmPosition_t *pxArmPos )
{
static xArmPosition_t xInputValue;             /* input from the queue */
static xArmPosition_t xInitialValue;
static xArmPosition_t xTempValue;
static xArmPosition_t xOffSetValues;

int i;
float baseRdiff = 0.0,
    baseEdiff = 0.0,
    elbowDiff = 0.0,
    wristPdiff = 0.0,
    wristRdiff = 0.0,
    grabDiff = 0.0;
    
    // Must have no block time as it will cause the mode change to have no effect
    if( pdTRUE == xQueueReceive( inputFromWPMQueue, &xInputValue, ( TickType_t ) 0 ) )
    {
         /* save in the shared resource */
        xInitialValue = *pxArmPos;
        //*pxArmPos = xInputValue;
        
        // caluculate difference multiplier
        baseRdiff = ((float)xInputValue.usBaseRotation - (float)pxArmPos->usBaseRotation) / (float)NUM_COEFFS;
        baseEdiff = ((float)xInputValue.usBaseElevation - (float)pxArmPos->usBaseElevation) / (float)NUM_COEFFS;
        elbowDiff = ((float)xInputValue.usElbow - (float)pxArmPos->usElbow) / (float)NUM_COEFFS;
        wristPdiff = ((float)xInputValue.usWristPitch - (float)pxArmPos->usWristPitch) / (float)NUM_COEFFS;
        wristRdiff = ((float)xInputValue.usWristRoll - (float)pxArmPos->usWristRoll) / (float)NUM_COEFFS;
        grabDiff = ((float)xInputValue.usGrabber - (float)pxArmPos->usGrabber) / (float)NUM_COEFFS; 
                
        //calculate the offset values
        xOffSetValues.usBaseElevation = fSMOOTHING_COEFFS[0] * baseEdiff;
        xOffSetValues.usBaseRotation = fSMOOTHING_COEFFS[0] * baseRdiff;
        xOffSetValues.usElbow = fSMOOTHING_COEFFS[0] * elbowDiff;
        xOffSetValues.usWristPitch = fSMOOTHING_COEFFS[0] * wristPdiff;
        xOffSetValues.usWristRoll = fSMOOTHING_COEFFS[0] * wristRdiff;
        xOffSetValues.usGrabber = fSMOOTHING_COEFFS[0] * grabDiff;
        
        for( i = 0; i < NUM_COEFFS; i++ )
        {
	        xTempValue.usBaseElevation = prvCalculateNewPosition( xInitialValue.usBaseElevation, xOffSetValues.usBaseElevation, baseEdiff, i );
            xTempValue.usBaseRotation = prvCalculateNewPosition( xInitialValue.usBaseRotation, xOffSetValues.usBaseRotation, baseRdiff, i );
	        xTempValue.usElbow = prvCalculateNewPosition( xInitialValue.usElbow, xOffSetValues.usElbow, elbowDiff, i );
            xTempValue.usWristPitch = prvCalculateNewPosition( xInitialValue.usWristPitch, xOffSetValues.usWristPitch, wristPdiff, i );
            xTempValue.usWristRoll = prvCalculateNewPosition( xInitialValue.usWristRoll, xOffSetValues.usWristRoll, wristRdiff, i );
            xTempValue.usGrabber = prvCalculateNewPosition( xInitialValue.usGrabber, xOffSetValues.usGrabber, grabDiff, i );
        
            /* when using HW disable interurpts */
            taskENTER_CRITICAL();
        
            pvWriteCompareFunctions[BaseElevation]( xTempValue.usBaseElevation );
            pvWriteCompareFunctions[BaseRotation]( xTempValue.usBaseRotation );
            pvWriteCompareFunctions[Elbow]( xTempValue.usElbow );
            pvWriteCompareFunctions[WristPitch]( xTempValue.usWristPitch );
            pvWriteCompareFunctions[WristRoll]( xTempValue.usWristRoll );
            pvWriteCompareFunctions[Grabber]( xTempValue.usGrabber );
        
            /* re-enable interrupts */
            taskEXIT_CRITICAL();
            
            //delay
            vTaskDelay(10);
        }
        
        *pxArmPos = xInputValue;
        vSetCurrentArmPosition( *pxArmPos );
        xSemaphoreGive( xRunCompleteSem );
       
    }
}

/*-----------------------------------------------------------------------*/
void prvOtherModeRx( xArmPosition_t *pxCurrentPosition )
{
static xServoQueueParams_t xInputValue;             /* input from the queue */
static uint16_t usNewValue = 0;
static uint16_t  ( *pusDirectionFunction ) ( uint16_t usLHS, uint16_t usRHS );

    // Must have no block time as it will cause the mode change to have no effect
    if( pdTRUE == xQueueReceive( inputFromDecoderTaskQueue, &xInputValue, ( TickType_t ) 0 ) )
        {
          
            /* Are we moving the servo left or right */
            switch( xInputValue.xDirection )
            {
                case SUB: pusDirectionFunction = &prvSub; break;
                case ADD: pusDirectionFunction = &prvAdd; break;
                default: break;
            }

            /* get the proposed new position of the correct servo */
            switch( xInputValue.xServo )
            {
                case BaseRotation:  usNewValue = pusDirectionFunction( pxCurrentPosition->usBaseRotation, usSERVO_SPEED );    break;
                case BaseElevation: usNewValue = pusDirectionFunction( pxCurrentPosition->usBaseElevation, usSERVO_SPEED );   break;
                case WristRoll:     usNewValue = pusDirectionFunction( pxCurrentPosition->usWristRoll, usSERVO_SPEED );       break;
                case Elbow:         usNewValue = pusDirectionFunction( pxCurrentPosition->usElbow, usSERVO_SPEED );           break;
                case WristPitch:    usNewValue = pusDirectionFunction( pxCurrentPosition->usWristPitch, usSERVO_SPEED );      break;
                case Grabber:       usNewValue = pusDirectionFunction( pxCurrentPosition->usGrabber, usSERVO_SPEED );         break;
                default: break;
            } 
            
            /*  Set the Duty Cycle to within limits, and not over the set period */
            if( ( usNewValue <= SERVO_MAX ) &&
                ( usNewValue >= SERVO_MIN ) &&
                ( usNewValue < usServoPeriod ) 
            )
            { 
                
                /* update the arm position since it's within limits */
                switch( xInputValue.xServo )
                {
                    case BaseRotation:  pxCurrentPosition->usBaseRotation = usNewValue;   break;
                    case BaseElevation: pxCurrentPosition->usBaseElevation = usNewValue;  break;
                    case WristRoll:     pxCurrentPosition->usWristRoll = usNewValue;      break;
                    case Elbow:         pxCurrentPosition->usElbow = usNewValue;          break;
                    case WristPitch:    pxCurrentPosition->usWristPitch = usNewValue;     break;
                    case Grabber:    
                        if( usNewValue >= GRAB_MAX )
                        {
                            pxCurrentPosition->usGrabber = usNewValue; 
                        }
                               
                    break;
                    default: break;
                } 
                
                /* save in the shared resource */
                vSetCurrentArmPosition( *pxCurrentPosition );
                
                /* when using HW disable interurpts */
                taskENTER_CRITICAL();
                
                pvWriteCompareFunctions[xInputValue.xServo]( usNewValue );
                
                 /* re-enable interrupts */
                taskEXIT_CRITICAL();
            }
        }
}

        
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vServoTask, pvParamaters )
{

( void ) pvParamaters;                       /* stops warnings */  

 xArmPosition_t xCurrentPosition = xGetCurrentPosition();

    /* the meat of the task */
    for (;;)
    {
        
        prvGetAndSend( &xCurrentPosition );
 
    } 
}

/*-----------------------------------------------------------------------*/
/* init */
void vStartServoTasks( int priority, xServoInputQueues_t *pxOutput )
{
    /* at this point the scheduler isn't runing so no need to enter critical here */
    
    /* all the servos have the same period so only need to read it once */
    usServoPeriod = baseElevationPWM_ReadPeriod();

    pvWriteCompareFunctions[BaseElevation] = &baseElevationPWM_WriteCompare;
    pvWriteCompareFunctions[BaseRotation] = &baseRotationPWM_WriteCompare;
    pvWriteCompareFunctions[Elbow] = &elbowPWM_WriteCompare;
    pvWriteCompareFunctions[WristPitch] = &wristPitchPWM_WriteCompare;
    pvWriteCompareFunctions[WristRoll] = &wristRollPWM_WriteCompare;
    pvWriteCompareFunctions[Grabber] = &grabberPWM_WriteCompare;

    inputFromDecoderTaskQueue = xQueueCreate( 10, sizeof( xServoQueueParams_t ) );
    inputFromWPMQueue = xQueueCreate( 10, sizeof( xArmPosition_t ) );
    
    pxOutput->pxFromKeypad = &inputFromDecoderTaskQueue;
    pxOutput->pxFromWPM = &inputFromWPMQueue;
    
    prvGetAndSend = &prvOtherModeRx;
    vSubscribeToModeChange( &prvModeChange );
    
    xRunCompleteSem = xSemaphoreCreateBinary();
    
    xTaskCreate( vServoTask, "ServoMove", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t* ) NULL);
    
}

uint16_t usGetServoInitPoint( xServoNumber_t xServo )
{
    uint16_t rc = SERVO_MID;
    
    switch ( xServo )
    {
        case BaseElevation:
        break;
        case BaseRotation:
        break;
        case WristPitch:
        break;
        case WristRoll:
        break;
        case Elbow:
        break;
        case Grabber:
        break;
        default:
        break;
    }
    
    
    return rc;
}
/*-----------------------------------------------------------------------*/
/* half way between the servo max value and it's minumum is gotten by 
* doing the min + (range/2), where range = man - min
*/
uint16_t usGetMidPoint( void )
{
    return SERVO_MID;
}

/*-----------------------------------------------------------------------*/
/* helper functions */
uint16_t prvAdd ( uint16_t usLHS, uint16_t usRHS )
{
    // dont bother checking for overflow
    return usLHS + usRHS;
}

uint16_t prvSub ( uint16_t usLHS, uint16_t usRHS )
{
    // dont bother checking for underflow
    return usLHS - usRHS;
}

/* [] END OF FILE */
