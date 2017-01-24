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

/* my own header */
#include "servo.h"

/* included to toggle the LED */
#include "partest.h"

/* to save the location */
#include "Custom Tasks/Current Position Store/currentposition.h"
#include "servoqueueparams.h"

#define SERVO_MAX 8192u    /* servo max value, calculated as per SSD, where the servo also seems to see 2.5 as 2ms */
#define SERVO_MIN 1638u    /* servo min value, calculated as per SSD, where the servo also seems to see 0.5 as 1ms */

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );
uint16_t usGetMidPoint( void );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t inputFromDecoderTaskQueue = NULL;
static QueueHandle_t inputFromWPMQueue = NULL;
static uint16_t usServoPeriod = 0;

/*-----------------------------------------------------------------------*/
/* Each value can either be added to or subtract from */
uint16_t prvAdd ( uint16_t usLHS, uint16_t usRHS );
uint16_t prvSub ( uint16_t usLHS, uint16_t usRHS );

/*-----------------------------------------------------------------------*/
/* function pointers to write compare */
void ( *pvWriteCompareFunctions[END] ) ( uint16_t newValue );

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
static const uint16_t usSERVO_SPEED = 50;           /* how far to move the servos, randomly chosen  */
( void ) pvParamaters;                              /* stops warnings */  
xServoQueueParams_t xInputValue;             /* input from the queue */
uint16_t usNewValue = 0;
uint16_t  ( *pusDirectionFunction ) ( uint16_t usLHS, uint16_t usRHS );

QueueHandle_t *pQueueToListenTo;    // This queue will either be the WPM or Decoder task queue, 
                                    // need to create a global mode variable to poll to check this, 
                                    // perhaps get the mode to send to everything that wants to be 
                                    // nofified of a change a notifacation, which the modules who 
                                    // care about it have to check with a timeout of 0

// not tested
arm_position_t xCurrentPosition = xGetCurrentPosition();

    /* the meat of the task */
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( inputFromDecoderTaskQueue, &xInputValue, portMAX_DELAY ) )
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
                case BaseRotation:  usNewValue = pusDirectionFunction( xCurrentPosition.usBaseRotation, usSERVO_SPEED );    break;
                case BaseElevation: usNewValue = pusDirectionFunction( xCurrentPosition.usBaseElevation, usSERVO_SPEED );   break;
                case WristRoll:     usNewValue = pusDirectionFunction( xCurrentPosition.usWristRoll, usSERVO_SPEED );       break;
                case Elbow:         usNewValue = pusDirectionFunction( xCurrentPosition.usElbow, usSERVO_SPEED );           break;
                case WristPitch:    usNewValue = pusDirectionFunction( xCurrentPosition.usWristPitch, usSERVO_SPEED );      break;
                case Grabber:       usNewValue = pusDirectionFunction( xCurrentPosition.usGrabber, usSERVO_SPEED );         break;
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
                    case BaseRotation:  xCurrentPosition.usBaseRotation = usNewValue;   break;
                    case BaseElevation: xCurrentPosition.usBaseElevation = usNewValue;  break;
                    case WristRoll:     xCurrentPosition.usWristRoll = usNewValue;      break;
                    case Elbow:         xCurrentPosition.usElbow = usNewValue;          break;
                    case WristPitch:    xCurrentPosition.usWristPitch = usNewValue;     break;
                    case Grabber:       xCurrentPosition.usGrabber = usNewValue;        break;
                    default: break;
                } 

                /* save in the shared resource */
                // not tested
                vSetCurrentArmPosition( xCurrentPosition );
            
                /* when using HW disable interurpts */
                taskENTER_CRITICAL();
                
                // not tested
                pvWriteCompareFunctions[xInputValue.xServo]( usNewValue );
                
                /* re-enable interrupts */
                taskEXIT_CRITICAL();
            }           
       
        }
        
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
    inputFromWPMQueue = xQueueCreate( 10, sizeof( xServoQueueParams_t ) );
    
    pxOutput->pxFromKeypad = &inputFromDecoderTaskQueue;
    pxOutput->pxFromWPM = &inputFromWPMQueue;
    
    xTaskCreate( vServoTask, "ServoMove", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t* ) NULL);
    
}

/*-----------------------------------------------------------------------*/
/* half way between the servo max value and it's minumum is gotten by 
* doing the min + (range/2), where range = man - min
*/
uint16_t usGetMidPoint( void )
{
    return SERVO_MIN + ( ( SERVO_MAX-SERVO_MIN ) / 2 );
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
