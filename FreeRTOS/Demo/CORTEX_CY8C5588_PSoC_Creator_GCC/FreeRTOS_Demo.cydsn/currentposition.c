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
#include "semphr.h"
#include <stdbool.h>

#include "currentposition.h"
#include "serial.h"

/************************************************************/
static uint8_t usCurrentPosition = 0;           /* shared resource */
static xSemaphoreHandle xGatekeeper = NULL;     /* mutex */
static bool xInitialised = false;               /* if the mutex is created this is true */

/************************************************************/
/* internal functions and a point to them mean I wotn have to repeat myself 
    when writing code to access the mutex etc
*/
void vGet( uint8_t* out );
void vSet( uint8_t* in );
void ( *funct ) ( uint8_t *usArg ); /* the fn pointer */

/************************************************************/
/* The generic mutex Take/Give function */

void vBase( void ( *funct ) ( uint8_t *usArg ), uint8_t *usArg )
{
     /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            funct( usArg );
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
            vBase( funct, usArg );
        }
    }  
}

/************************************************************/
/* inward facing getter */
void vGet( uint8_t *usOut )
{
    *usOut = usCurrentPosition;
}

/************************************************************/
/* inward facing setter */
void vSet( uint8_t *usIn )
{
    usCurrentPosition = *usIn;
}

/************************************************************/
/* external facing setter */
void vSetCurrentPosition( uint8_t usNewPosition )
{
    vBase( &vSet, &usNewPosition );
}

/************************************************************/
/* external facing getter */
uint8_t usGetCurrentPosition( void )
{
uint8_t rc = 0xFF; /* Initialise to an out of range value */
    
    vBase( &vGet, &rc );
    return rc;
}

/* [] END OF FILE */
