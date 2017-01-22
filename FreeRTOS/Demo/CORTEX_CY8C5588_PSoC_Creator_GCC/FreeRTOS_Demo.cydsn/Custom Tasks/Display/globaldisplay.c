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
void prvVGetCom( xComPortHandle *out );
void prvVSetCom( xComPortHandle *in );
void prvVComBase( void ( *comFunct ) ( xComPortHandle *xCom ), xComPortHandle *xArg );

void prvVGetTask( xTaskHandle *out );
void prvVSetTask( xTaskHandle *in );
void prvVTaskBase( void ( *taskFunct ) ( xTaskHandle *xTask ), xTaskHandle *xArg );

void prvVGetQ( QueueHandle_t *out );
void prvVSetQ( QueueHandle_t *in );
void prvVQBase( void ( *QFunct ) ( QueueHandle_t *xCom ), QueueHandle_t *xArg );

/*-----------------------------------------------------------------------------*/
void prvVSetQ( QueueHandle_t *in )
{
    xDispQueue = *in;
}

/*-----------------------------------------------------------------------------*/
void prvVGetQ( QueueHandle_t *out )
{
    *out = xDispQueue;
}

/*-----------------------------------------------------------------------------*/
void prvVSetTask( xTaskHandle *in )
{
    xComTask = *in;
}

/*-----------------------------------------------------------------------------*/
void prvVGetTask( xTaskHandle *out )
{
    *out = xComTask;
}

/*-----------------------------------------------------------------------------*/
void prvVGetCom( xComPortHandle *out )
{
    *out = xComPort;
}

/*-----------------------------------------------------------------------------*/
void prvVSetCom( xComPortHandle *in )
{
    xComPort = *in;
}

/*-----------------------------------------------------------------------------*/
void prvVComBase( void ( *comFunct ) ( xComPortHandle *xCom ), xComPortHandle *xArg )
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
            prvVComBase( comFunct, xArg );
        }
    } 
}

/*-----------------------------------------------------------------------------*/
void prvVTaskBase( void ( *taskFunct ) ( xTaskHandle *xTask ), xTaskHandle *xArg )
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
            prvVTaskBase( taskFunct, xArg );
        }
    } 
}

/*-----------------------------------------------------------------------------*/
void prvVQBase( void ( *QFunct ) ( QueueHandle_t *xCom ), QueueHandle_t *xArg )
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
            prvVQBase( QFunct, xArg );
        }
    }      
}

/*------------------------------------------------------------------*/
xTaskHandle xGetDisplayTaskHandle( void )
{
    xTaskHandle rc;
    prvVTaskBase( &prvVGetTask, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
QueueHandle_t xGetDisplayInputQueue( void )
{
    QueueHandle_t rc;
    prvVQBase( &prvVGetQ, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
xComPortHandle xGetDisplayComPortHandle( void )
{
    xComPortHandle rc;
    prvVComBase( &prvVGetCom, &rc );
    return rc;
}

/*------------------------------------------------------------------*/
void vSetDisplayTaskHandle( xTaskHandle xNewHandle )
{
    prvVTaskBase( &prvVSetTask, &xNewHandle );  
}

/*------------------------------------------------------------------*/
void vSetDisplayInputQueue( QueueHandle_t xNewQueue )
{
    prvVQBase( &prvVSetQ, &xNewQueue );
}

/*------------------------------------------------------------------*/
void vSetDisplayComPortHandle( xComPortHandle xNewHandle )
{
    prvVComBase( &prvVSetCom, &xNewHandle );
}
    
/* [] END OF FILE */
