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

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t outputQueue = NULL;

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
    
( void ) pvParamaters;              /* stops warnings */
uint8_t inputValue = 0;             /* input from the queue */
const uint8_t SERVO_MIN = 0x00;     /* servo min value, this will be changed at some point so now it's more of a placeholder */
const uint8_t SERVO_MAX = 0xFF;     /* servo max value, this will be changed at some point so now it's more of a placeholder */

    // the meat of the task
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( outputQueue, &inputValue, portMAX_DELAY ) )
        {
            
            /* when using HW disable interurpts */
            taskENTER_CRITICAL();
            
            /* for now this test is pointless, however when i actually know these limits
            it's easy to change the values in the variables. Set the Duty Cycle to within limits, and not over the set period */
            if( ( inputValue <= SERVO_MAX ) &&
                ( inputValue > SERVO_MIN ) &&
                ( inputValue < servoPWM_ReadPeriod() ) 
            )
            { 
                servoPWM_WriteCompare( inputValue ); 
            }
            
            /* re-enable interrupts */
            taskEXIT_CRITICAL();
        }
        
    }
    
}
/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t* xStartServoTasks( int priority )
{

    outputQueue = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(uint8));
    xTaskCreate( vServoTask, "Servo", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t *) NULL);
    
    return &outputQueue;
}

/* [] END OF FILE */
