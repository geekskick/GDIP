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

/*-----------------------------------------------------------------------------*/
/* internal functions and a point to them mean I wont have to repeat myself 
    when writing code to access the mutex etc
*/
void vGetCom( xComPortHandle *out );
void vSetCom( xComPortHandle *in );
void vComBase( void ( *comFunct ) ( xComPortHandle *xCom ), xComPortHandle *xArg );

void vGetTask( xTaskHandle *out );
void vSetTask( xTaskHandle *in );
void vTaskBase( void ( *taskFunct ) ( xTaskHandle *xTask ), xTaskHandle *xArg );

void vGetQ( QueueHandle_t *out );
void vSetQ( QueueHandle_t *in );
void vQueueBase( void ( *QFunct ) ( QueueHandle_t *xCom ), QueueHandle_t *xArg );

/*-----------------------------------------------------------------------------*/
void vComBase( void ( *comFunct ) ( xComPortHandle *xCom ), xComPortHandle *xArg )
{
    /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            comFunct( xArg );
            xSemaphoreGive( xGatekeeper );
        }
    }
    else 
    {
        /* here it isn't initialised so initialise the mutex, and if successful recursively
        attempt to perform the action again */
        xGatekeeper = xSemaphoreCreateMutex();
        if( xGatekeeper != NULL )
        {
            xInitialised = true;
            vQueueBase( comFunct, xArg );
        }
    } 
}

/*-----------------------------------------------------------------------------*/
void vTaskBase( void ( *taskFunct ) ( xTaskHandle *xTask ), xTaskHandle *xArg )
{
    /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            taskFunct( xArg );
            xSemaphoreGive( xGatekeeper );
        }
    }
    else 
    {
        /* here it isn't initialised so initialise the mutex, and if successful recursively
        attempt to perform the action again */
        xGatekeeper = xSemaphoreCreateMutex();
        if( xGatekeeper != NULL )
        {
            xInitialised = true;
            vQueueBase( taskFunct, xArg );
        }
    } 
}

/*-----------------------------------------------------------------------------*/
void vQueueBase( void ( *QFunct ) ( QueueHandle_t *xCom ), QueueHandle_t *xArg )
{
    /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            QFunct( xArg );
            xSemaphoreGive( xGatekeeper );
        }
    }
    else 
    {
        /* here it isn't initialised so initialise the mutex, and if successful recursively
        attempt to perform the action again */
        xGatekeeper = xSemaphoreCreateMutex();
        if( xGatekeeper != NULL )
        {
            xInitialised = true;
            vQueueBase( QFunct, xArg );
        }
    }      
}

/*------------------------------------------------------------------*/
xTaskHandle xGetDisplayTaskHandle( void )
{
    xTaskHandle rc;
    vTaskBase( &vGetTask, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
QueueHandle_t xGetDisplayInputQueue( void )
{
    QueueHandle_t rc;
    vQueueBase( &vGetQ, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
xComPortHandle xGetDisplayComPortHandle( void )
{
    xComPortHandle rc;
    vComBase( &vGetCom, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
void vSetDisplayTaskHandle( xTaskHandle xNewHandle )
{
    vTaskBase( &vSetTask, &xNewHandle );  
}

/*------------------------------------------------------------------*/
void vSetDisplayInputQueue( QueueHandle_t xNewQueue )
{
    vQueueBase( &vSetQ, &xNewQueue );
}

/*------------------------------------------------------------------*/
void vSetDisplayComPortHandle( xComPortHandle xNewHandle )
{
    vComBase( &vSetCom, &xNewHandle );
    
}
    
/* [] END OF FILE */
