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
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "serial.h"
#include "semphr.h"

#include "globaldisplay.h"
  
static xSemaphoreHandle xGatekeeper = NULL;     /* mutex */
static bool xInitialised = false;               /* if the mutex is created this is true */
#include "semphr.h"
#include <stdbool.h>

static QueueHandle_t xDispQueue;
static xComPortHandle xComPort;
static xTaskHandle xComTask;

/************************************************************/
/* internal functions and a point to them mean I wont have to repeat myself 
    when writing code to access the mutex etc
*/
void vGetCom( xComPortHandle *out );
void vSetCom( xComPortHandle *in );

void vGetTask( xTaskHandle *out );
void vSetTask( xTaskHandle *in );

void vGetQ( QueueHandle_t *out );
void vSetQ( QueueHandle_t *in );

void vComBase( void ( *comFunct ) ( xComPortHandle *xCom ), xComPortHandle *xArg );
void vTaskBase( void ( *taskFunct ) ( xTaskHandle *xTask ), xTaskHandle *xArg );
void vQueueBase( void ( *QFunct ) ( QueueHandle_t *xCom ), QueueHandle_t *xArg );

/*------------------------------------------------------------------*/
xTaskHandle xGetDisplayTaskHandle( void )
{
    xTaskHandle rc;
    
    return rc;
}

/*------------------------------------------------------------------*/
QueueHandle_t xGetDisplayInputQueue( void )
{
    QueueHandle_t rc;
    return rc;
}

/*------------------------------------------------------------------*/
xComPortHandle xGetDisplayComPortHandle( void )
{
    xComPortHandle rc;
    
    return rc;
}

/*------------------------------------------------------------------*/
void vSetDisplayTaskHandle( xTaskHandle xNewHandle )
{
    ( void ) xNewHandle;
    
}

/*------------------------------------------------------------------*/
void vSetDisplayInputQueue( QueueHandle_t xNewQueue )
{
    ( void ) xNewQueue;
}

/*------------------------------------------------------------------*/
void vSetDisplayComPortHandle( xComPortHandle xNewHandle )
{
    ( void )xNewHandle;
    
}
    
/* [] END OF FILE */
