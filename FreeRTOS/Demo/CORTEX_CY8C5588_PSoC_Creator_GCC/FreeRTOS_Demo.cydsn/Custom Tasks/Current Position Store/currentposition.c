/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

/* Scheduler include files. */
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>

#include "currentposition.h"

/************************************************************/
static uint16_t usCurrentPosition = 0;          /* shared resource */
static xSemaphoreHandle xGatekeeper = NULL;     /* mutex */
static bool xInitialised = false;               /* if the mutex is created this is true */

static  xArmPosition_t xCurrentPosition;

/************************************************************/
/* internal functions and a point to them mean I wont have to repeat myself 
    when writing code to access the mutex etc
*/
void vGet( uint16_t* out );
void vSet( uint16_t* in );

/************************************************************/
/* internal functions for enitre arm set up 
and a point to them mean I wotn have to repeat myself 
    when writing code to access the mutex etc
*/
void vGetArmPosition(  xArmPosition_t* pxOut );
void vSetArmPosition(  xArmPosition_t* pxIn );

/************************************************************/
/* The generic mutex Take/Give function */

void vBase( void ( *funct ) ( uint16_t *usArg ), uint16_t *usArg )
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
/* The generic mutex Take/Give function */

void vBaseFunction( void ( *positionFunct ) (  xArmPosition_t *pxArg ),  xArmPosition_t *pxArg )
{
     /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            positionFunct( pxArg );
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
            vBaseFunction( positionFunct, pxArg );
        }
    }  
}
/************************************************************/
/* inward facing getter */
void vGet( uint16_t *usOut )
{
    *usOut = usCurrentPosition;
}

void vGetArmPosition(  xArmPosition_t* pxOut )
{
    *pxOut = xCurrentPosition;
}

/************************************************************/
/* inward facing setter */
void vSet( uint16_t *usIn )
{
    usCurrentPosition = *usIn;
}

void vSetArmPosition(  xArmPosition_t* pxIn )
{
    xCurrentPosition = *pxIn;
}

/************************************************************/
/* external facing setter */
void vSetCurrentPosition( uint16_t usNewPosition )
{
    vBase( &vSet, &usNewPosition );
}

void vSetCurrentArmPosition(  xArmPosition_t xNewPosition )
{
	vBaseFunction( &vSetArmPosition, &xNewPosition );
}

/************************************************************/
/* external facing getter */
uint16_t usGetCurrentPosition( void )
{
uint16_t rc = 0xFFFF; /* Initialise to an out of range value */
    
    vBase( &vGet, &rc );
    return rc;
}

 xArmPosition_t xGetCurrentPosition( void )
{
 xArmPosition_t rc;

	vBaseFunction( &vGetArmPosition ,&rc );
	return rc;
}

/* [] END OF FILE */
