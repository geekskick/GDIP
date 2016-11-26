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

#include "ServoQueueParams.h"
#include "decodertask.h"
#include "partest.h"

QueueHandle_t xOutputQueue = NULL;
QueueHandle_t xKeypadInputQueue = NULL;

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION_PROTO( vDecoderTask, pvParameters );

static portTASK_FUNCTION( vDecoderTask, pvParamaters )
{
char8 cButton;
uint8_t usSend = 0;
struct xServoQueueParams xToServo;
    
    struct xDecoderParams xParams = *( (struct xDecoderParams* ) pvParamaters );
    ( void ) xParams; /* get rid of warnings */
    for(;;)
    {
        if( pdTRUE == xQueueReceive( xKeypadInputQueue, &cButton, portMAX_DELAY ) )
        {
            xToServo.usServoNumber = 1;
            
            switch( cButton )
            {
                case 'a':
                    xToServo.xDirection = ADD;
                    usSend = 1;
                    break;
                case 'b':
                    xToServo.xDirection = SUB;
                    usSend = 1;
                    break;
                default:
                    usSend = 0;
                    break;
                
            }
            
            if( usSend == 1 )
                {
                if( pdFALSE == xQueueSend( xOutputQueue, ( void* )&xToServo, ( TickType_t ) 100 ) )
                {
               
                }
            }
        }
        //vParTestToggleLED(1);
    }
}

/*-----------------------------------------------------------------------*/
QueueHandle_t xStartDecoderTask( int priority, struct xDecoderParams xParams )
{
    xKeypadInputQueue = xParams.xKeypadQueue;
    xTaskCreate( vDecoderTask, "Decoder", configMINIMAL_STACK_SIZE, ( void * ) &xParams, priority, NULL );
    xOutputQueue = xQueueCreate( 10, sizeof( struct xServoQueueParams ) );
    return xOutputQueue;
}

/* [] END OF FILE */
