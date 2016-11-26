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
#include "currentposition.h"
#include "servoqueueparams.h"

#define SERVO_MAX 8192u    /* servo max value, calculated as per SSD, where the servo also seems to see 2.5 as 2ms */
#define SERVO_MIN 1638u    /* servo min value, calculated as per SSD, where the servo also seems to see 0.5 as 1ms */

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t inputQueue = NULL;
static uint16_t usServoPeriod = 0;

/*-----------------------------------------------------------------------*/
/* half way between the servo max value and it's minumum is gotten by 
* doing the min + (range/2), where range = man - min
*/
uint16_t usGetMidPoint( void )
{
    return SERVO_MIN + ( ( SERVO_MAX-SERVO_MIN ) / 2 );
}

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
static const uint16_t ulSERVO_SPEED = 50;          /* how far to move the servos, randomly chosen  */
( void ) pvParamaters;                              /* stops warnings */  
struct xServoQueueParams xInputValue;               /* input from the queue */
uint16_t ulCurrentVal = usGetCurrentPosition();     /* the current servo position needs to be initialised before it's modified by the add/subtract */

    /* the meat of the task */
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( inputQueue, &xInputValue, portMAX_DELAY ) )
        {
            /* for now arbitrarily move the servo by a value, I need to find a way of making compare 
            values to degrees of servo movement */
            if( ADD == xInputValue.xDirection )
            {
                ulCurrentVal += ulSERVO_SPEED;
            }
            else if( SUB == xInputValue.xDirection )
            {
                ulCurrentVal -= ulSERVO_SPEED;   
            }  
            
            /*  Set the Duty Cycle to within limits, and not over the set period */
            if( ( ulCurrentVal <= SERVO_MAX ) &&
                ( ulCurrentVal >= SERVO_MIN ) &&
                ( ulCurrentVal < usServoPeriod ) 
            )
            { 
                /* save in the shared resource */
                vSetCurrentPosition( ulCurrentVal );
            
                /* when using HW disable interurpts */
                taskENTER_CRITICAL();
                
                servoPWM_WriteCompare( ulCurrentVal );
                
                /* re-enable interrupts */
                taskEXIT_CRITICAL();
            }           
       
        }
        
    }
    
}
/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t xStartServoTasks( int priority, QueueHandle_t xInputQueue )
{

    /* at this point the scheduler isn't runing so no need to enter critical here */
    usServoPeriod = servoPWM_ReadPeriod();

    inputQueue = xInputQueue;
    xTaskCreate( vServoTask, "ServoMove", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t* ) NULL);
    
    return inputQueue;
}

/* [] END OF FILE */
