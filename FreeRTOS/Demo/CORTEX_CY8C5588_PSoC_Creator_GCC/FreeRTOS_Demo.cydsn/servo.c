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

/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );

/* the queue which the task will receive from */
static QueueHandle_t servoQueue = NULL;

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
    /* stops warnings */
    ( void ) pvParamaters; 
    
    uint8_t inputValue = 0;
    const uint8_t SERVO_MIN = 6, 
                  SERVO_MAX = 25;
    
    // the meat of the task
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( servoQueue, &inputValue, portMAX_DELAY ) )
        {
            /* The LED should tun off immediately */
            vParTestToggleLED(0);
            
            /* when using HW disable interurpts */
            taskENTER_CRITICAL();
            
            /* for now this test is pointless, however when i actually know these limits
            it's easy to change the values in the variables. Set the Duty Cycle to within limits, and not over the set period */
            if( ( inputValue <= SERVO_MAX ) &&
                ( inputValue >= SERVO_MIN ) &&
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

QueueHandle_t* xStartServoTasks( int priority )
{
    /* create the queue 
    100 bytes should be quite enough */
    servoQueue = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(uint8));
    
    /* create the task */
    xTaskCreate( vServoTask, "Servo", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t *) NULL);
    
    return &servoQueue;
}

/* [] END OF FILE */
