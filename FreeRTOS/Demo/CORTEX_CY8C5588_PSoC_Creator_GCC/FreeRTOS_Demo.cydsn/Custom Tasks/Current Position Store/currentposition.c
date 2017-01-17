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
static uint16_t usCurrentPosition = 0;           /* shared resource */
static xSemaphoreHandle xGatekeeper = NULL;     /* mutex */
static bool xInitialised = false;               /* if the mutex is created this is true */

static arm_position_t xCurrentPositon = 0;

/************************************************************/
/* internal functions and a point to them mean I wotn have to repeat myself 
    when writing code to access the mutex etc
*/
void vGet( uint16_t* out );
void vSet( uint16_t* in );
void ( *funct ) ( uint16_t *usArg ); /* the fn pointer */

/************************************************************/
/* internal functions for enitre arm set up 
and a point to them mean I wotn have to repeat myself 
    when writing code to access the mutex etc
*/
void vGetArmPosition( arm_position_t* pxOut );
void vSetArmPosition( arm_position_t* pxIn );
void ( *positonFunct_t ) ( arm_position_t *xArg ); /* the fn pointer */

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

void vBaseFunction( void ( *positionFunct_t ) ( arm_position_t *pxArg ), arm_position_t *pxArg )
{
     /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            postionFunct_t( pxArg );
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
            vBase( positionFunct_t, pxArg );
        }
    }  
}
/************************************************************/
/* inward facing getter */
void vGet( uint16_t *usOut )
{
    *usOut = usCurrentPosition;
}

void vGetArmPosition( arm_position_t* pxOut )
{
    *pxOut = xCurrentPosition;
}

/************************************************************/
/* inward facing setter */
void vSet( uint16_t *usIn )
{
    usCurrentPosition = *usIn;
}

void vSetArmPosition( arm_position_t* pxIn )
{
    xCurrentPosition = *pxIn;
}

/************************************************************/
/* external facing setter */
void vSetCurrentPosition( uint16_t usNewPosition )
{
    vBase( &vSet, &usNewPosition );
}

void vSetCurrentArmPosition( arm_position_t xNewPosition )
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

arm_position_t xGetCurrentPosition( void )
{
arm_position_t rc;

	vBaseFunction( &vGetArmPosition ,&rc );
	return rc;
}

/* [] END OF FILE */
