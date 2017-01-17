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

#include "WPM.h"
#include "currentposition.h" // This file contains the arm position struct definition

// The number of positions the user can store and therefore the depth of the stack
#define SAVED_POSITIONS_MAX 10

// Wait for 200ms before timing out while waiting for notifications
#define NOTIFY_WAIT_TIM_OUT 200 

/*-----------------------------------------------------------------------*/
/* The arm can move through the saved way points either forwards or backwards */
typedef enum directions { FORWARD, BACK } direction_t;
typedef enum actions	{ STOP, SAVE, RESET, RUN, CLEAR, NONE } action_t;

struct action_args{
	action_t 	*pxNextAction;
	direction_t *pxCurrentDirection;
	uint8_t *pusNextFreeStackPosition;
	uint8_t *pusCurrentStackPosition;
}

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vWPMTask, pvParameters );

/*-----------------------------------------------------------------------*/
/* The action functions */
static void wpm_action_stop( struct action_args args );
static void wpm_action_save( struct action_args args );
static void wpm_action_clear( struct action_args args );
static void wpm_action_reset( struct action_args args );
static void wpm_action_run( struct action_args args );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
static QueueHandle_t outputQueue = NULL;

/*-----------------------------------------------------------------------*/
/* The stack of saved positions */
static arm_position_t[SAVED_POSITIONS_MAX] xStack = { 0, };

/*-----------------------------------------------------------------------*/
/* the main function reads from the queue and sets the PWM duty  to the passed in value */
static portTASK_FUNCTION( vWPMTask, pvParamaters )
{

( void ) pvParamaters;								/* stops warnings */  
uint32_t uNotificationValue = WPM_NOTIFICATION_NONE;	

direction_t xDirection = FORWARD;
action_t xNextAction = NONE;

uint8_t usNextFreeStackPosition = 0;
		usCurrentStackPosition = 0;

bool bNotifcationRxd = false;

struct action_args action_package;
action_package.pxNextAction = &xNextAction;
action_package.pxCurrentDirection = &xDirection;
action_package.pusCurrentStackPosition = &usCurrentStackPosition;
action_package.pusNextFreeStackPosition = &usNextFreeStackPosition

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
        			wpm_action_stop( action_package );
        			break;

        		case WPM_NOTIFICATION_SAVE:
        			wpm_action_sacve( action_package );
        			break;

        		case WPM_NOTIFICATION_RESET:
        			wpm_action_reset( action_package );
        			break;

        		case WPM_NOTIFICATION_RUN:
        			xDirection = FORWARD;
        			wpm_action_run( action_package );
        			break;

        		case WPM_NOTIFICATION_CLEAR:
        			wpm_action_clear( action_package );
        			break;

        		default:
        			/* unknown notifcation rx'd */
        			break;
        	}


        } 
        else if( xNextAction == RUN)
        {
        	wpm_action_run( action_package );
        }
        
    }
    
}

/*-----------------------------------------------------------------------*/
static void wpm_action_stop( struct action_args args )
{
	args.pxNextAction = NONE;
}

/*-----------------------------------------------------------------------*/
static void wpm_action_reset( struct action_args args )
{
	args.pxCurrentDirection = BACK;
	args.pxNextAction = RUN;
}

/*-----------------------------------------------------------------------*/
static void wpm_action_save( struct action_args args )
{
	arm_position_t xCurrentPosition; 
	// xCurrentPosition = getCurrentPos(); something like this

	// Prevent overflow
	if( *( args.pusNextFreeStackPosition ) != SAVED_POSITIONS_MAX )
	{
		xStack[*( args.pusNextFreeStackPosition )++] = xCurrentPosition;
	}
	else
	{
		// if it gets here then the stack is full
	}
	args.pxNextAction = NONE;
}

/*-----------------------------------------------------------------------*/
static void wpm_action_clear( struct action_args args )
{

	// Prevent underflow
	if( *( args.pusNextFreeStackPosition ) > 0 )
	{
		xStack[*( args.pusNextFreeStackPosition )--] = NULL;
	}
	else
	{
		// if it gets here then the stack is aldready empty
	}
	args.pxNextAction = NONE;
}

/*-----------------------------------------------------------------------*/
static void wpm_action_run( struct action_args args )
{
	int16_t sNextStack = *(args.pusCurrentStackPosition) + ( *( args.pxCurrentDirection ) == FORWARD ? 1 : -1 ); 

	if( sNextStack >= 0 && sNextStack < SAVED_POSITIONS_MAX )
	{
		*( args.pusCurrentStackPosition ) = ( uint8_t )sNextStack;
		
		// Write to the output queue here
	}
	else
	{
		*args.pxNextAction = NONE;
	}
}

/*-----------------------------------------------------------------------*/
/* init */
QueueHandle_t xStartWPMTask( int priority, QueueHandle_t xOutputQueue )
{
    outputQueue = xOutputQueue;

    //this stack size will need changing as it needs more room for the stack of positions
    return xTaskCreate( vWPMTask, "WPM", configMINIMAL_STACK_SIZE, ( void* ) NULL , priority, ( TaskHandle_t* ) NULL);
}

