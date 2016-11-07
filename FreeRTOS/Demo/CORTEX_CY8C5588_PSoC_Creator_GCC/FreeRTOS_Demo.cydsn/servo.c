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

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
    /* get out the paramters */
    struct servoArgs *args = ( ( struct servoArgs* ) pvParamaters );
    uint8_t inputValue = 0;
    const uint8_t SERVO_MIN = 0x00, 
                  SERVO_MAX = 0xFF;
    
    // the meat of the task
    for (;;)
    {
        /* block forever to get the value from the queue */
        if( pdTRUE == xQueueReceive( args->inputQueue, &inputValue, portMAX_DELAY ) )
        {
            /* The LED should tun off immediately */
            vParTestToggleLED(0);
            
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

void vStartServoTasks(void *pvParams, int priority)
{
    
    xTaskCreate( vServoTask, "Servo", configMINIMAL_STACK_SIZE, pvParams, priority, ( TaskHandle_t *) NULL);
}

/* [] END OF FILE */
