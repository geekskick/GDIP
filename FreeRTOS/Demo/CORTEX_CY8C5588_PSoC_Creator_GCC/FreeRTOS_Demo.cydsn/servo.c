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

#define SERVO_MAX 25    /* servo max value, obtained from trial and error */
#define SERVO_MIN 6     /* servo min value, obtained from trial and error */

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t outputQueue = NULL;
static uint8_t usServoPeriod = 0;

/*-----------------------------------------------------------------------*/
/* half way between the servo max value and it's minumum is gotten by 
* doing the min + (range/2), where range = man - min
*/
uint8_t usGetMidPoint( void )
{
    return SERVO_MIN + ( ( SERVO_MAX-SERVO_MIN ) / 2 );
}

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
    
( void ) pvParamaters;              /* stops warnings */
uint8_t inputValue = 0;             /* input from the queue */

    // the meat of the task
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( outputQueue, &inputValue, portMAX_DELAY ) )
        {
            
            /*  Set the Duty Cycle to within limits, and not over the set period */
            if( ( inputValue <= SERVO_MAX ) &&
                ( inputValue >= SERVO_MIN ) &&
                ( inputValue < usServoPeriod ) 
            )
            { 
                /* save in the shared resource */
                vSetCurrentPosition( inputValue );
            
                /* when using HW disable interurpts */
                taskENTER_CRITICAL();
                
                servoPWM_WriteCompare( inputValue );
                
                /* re-enable interrupts */
                taskEXIT_CRITICAL();
            }
            
       
        }
        
    }
    
}
/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t* xStartServoTasks( int priority )
{

    /* at this point the scheduler isn't runing so no need to enter critical here */
    usServoPeriod = servoPWM_ReadPeriod();

    outputQueue = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(uint8));
    xTaskCreate( vServoTask, "ServoMove", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t *) NULL);
    
    return &outputQueue;
}

/* [] END OF FILE */
