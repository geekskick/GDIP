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
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "WPM.h"
#include "Custom Tasks/Current Position Store/currentposition.h"
//#include "currentposition.h" // This file contains the arm position struct definition

// The number of positions the user can store and therefore the depth of the stack
#define SAVED_POSITIONS_MAX 10

// Wait for 200ms before timing out while waiting for notifications
#define NOTIFY_WAIT_TIM_OUT 200 

/*-----------------------------------------------------------------------*/
/* The arm can move through the saved way points either forwards or backwards */
typedef enum directions { FORWARD, BACK } direction_t;
typedef enum actions	{ STOP, SAVE, RESET, RUN, CLEAR, NONE } action_t;

struct xActionArgs{
	action_t 	*pxNextAction;
	direction_t *pxCurrentDirection;
	uint8_t *pusNextFreeStackPosition;
	uint8_t *pusCurrentStackPosition;
};

/* The stack items are not pointers so it can't be set to NULL etc,
 * so if the stack item is used then it'll be is_used = true 
 */
struct stack_item_t{
    arm_position_t arm_position;
    bool is_used;
};

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vWPMTask, pvParameters );

/*-----------------------------------------------------------------------*/
/* The action functions */
void prvActionStop( struct xActionArgs args );
void prvActionSave( struct xActionArgs args );
void prvActionClear( struct xActionArgs args );
void prvActionReset( struct xActionArgs args );
void prvActionRun( struct xActionArgs args );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
QueueHandle_t xWPMOutputQueue = NULL;

/*-----------------------------------------------------------------------*/
/* The stack of saved positions */
static struct stack_item_t xStack[SAVED_POSITIONS_MAX];

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vWPMTask, pvParamaters )
{

( void ) pvParamaters;								/* stops warnings */  
uint32_t uNotificationValue = WPM_NOTIFICATION_NONE;	

direction_t xDirection = FORWARD;
action_t xNextAction = NONE;

uint8_t usNextFreeStackPosition = 0,
		usCurrentStackPosition = 0;

bool bNotifcationRxd = false;

struct xActionArgs xActionPckg;
xActionPckg.pxNextAction = &xNextAction;
xActionPckg.pxCurrentDirection = &xDirection;
xActionPckg.pusCurrentStackPosition = &usCurrentStackPosition;
xActionPckg.pusNextFreeStackPosition = &usNextFreeStackPosition;

    /* the meat of the task */
    for (;;)
    {
        //wait for the notification
        // bNotificationRxd == xTaskNotifyWait( something here );

        if( pdTRUE == bNotifcationRxd )
        {
        	switch( uNotificationValue )
        	{
        		case WPM_NOTIFICATION_STOP:
        			prvActionStop( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_SAVE:
        			prvActionSave( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_RESET:
        			prvActionReset( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_RUN:
        			xDirection = FORWARD;
        			prvActionRun( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_CLEAR:
        			prvActionClear( xActionPckg );
        			break;

        		default:
        			/* unknown notifcation rx'd */
        			break;
        	}


        } 
        else if( xNextAction == RUN)
        {
        	prvActionRun( xActionPckg );
        }
        
    }
    
}

/*-----------------------------------------------------------------------*/
void prvActionStop( struct xActionArgs args )
{
	*( args.pxNextAction ) = NONE;
}

/*-----------------------------------------------------------------------*/
void prvActionReset( struct xActionArgs args )
{
	*( args.pxCurrentDirection ) = BACK;
	*( args.pxNextAction ) = RUN;
}

/*-----------------------------------------------------------------------*/
void prvActionSave( struct xActionArgs args )
{
	arm_position_t xCurrentPosition; 
	// xCurrentPosition = getCurrentPos(); something like this

	// Prevent overflow
	if( *( args.pusNextFreeStackPosition ) != SAVED_POSITIONS_MAX )
	{
        if( !xStack[*( args.pusNextFreeStackPosition )].is_used )
        {
        /* save the arm position then move to the next high up item in the stack */
		xStack[*( args.pusNextFreeStackPosition )].arm_position = xCurrentPosition;
        xStack[*( args.pusNextFreeStackPosition )].is_used = true;
        *( args.pusNextFreeStackPosition ) += 1;
        }
        else
        {
            // if it gets here then there is an error with the stack positioning
            // and something is used but the index pointers think otherwise
        }
	}
	else
	{
		// if it gets here then the stack is full
	}
	*( args.pxNextAction ) = NONE;
}

/*-----------------------------------------------------------------------*/
void prvActionClear( struct xActionArgs args )
{

	// Prevent underflow
	if( *( args.pusNextFreeStackPosition ) > 0 )
	{
        /* mark it as not used then decrement the pointer value */
		xStack[*( args.pusNextFreeStackPosition )].is_used = false;
        *( args.pusNextFreeStackPosition ) -= 1;
	}
	else
	{
		// if it gets here then the stack is already empty
	}
	*( args.pxNextAction ) = NONE;
}

/*-----------------------------------------------------------------------*/
void prvActionRun( struct xActionArgs args )
{
    // ge tthe next stack item index,might be an error if this next item is unused? suck it and see
	int16_t sNextStack = *(args.pusCurrentStackPosition) + ( *( args.pxCurrentDirection ) == FORWARD ? 1 : -1 ); 

    // range check
	if( sNextStack >= 0 && sNextStack < SAVED_POSITIONS_MAX )
	{
		*( args.pusCurrentStackPosition ) = ( uint8_t )sNextStack;
		
        if( xStack[*( args.pusCurrentStackPosition )].is_used )
        {
            // Write to the output queue here
            
        }
		else 
        {
            // not at the top of the max stack items but at the top of the currently allocated slots
            // probably do nothing here
        }
        
	}
	else
	{
        // stop in case of reaching start/end of stack
		*args.pxNextAction = NONE;
	}
}

/*-----------------------------------------------------------------------*/
/* init */
TaskHandle_t xStartWPMTask( int priority, xWPMParams_t xParams )
{
    xWPMOutputQueue = *( xParams.pxServoInputQueue );
    TaskHandle_t rc;
    //this stack size will need changing as it needs more room for the stack of positions
    xTaskCreate( vWPMTask, "WPM", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t* ) &rc );
    return rc;
}

