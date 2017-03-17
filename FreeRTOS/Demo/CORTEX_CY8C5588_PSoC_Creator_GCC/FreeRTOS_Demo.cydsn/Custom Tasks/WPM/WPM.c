/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 3
Date           : 12th Feb 2017
Changes Made   : 
    Waypoints stored in a giant array
    as dynamic memory allocation is
    abstracted away by freertos. 
    Clearing a waypoint, and stop 
    not tested
*****************************************
Change ID      : 10
Version        : 4
Date           : 2nd March 2017
Changes Made   : 
    wpmClear decrements the current pos
    index and marks the last used
    as free. Issue#9
*****************************************/

/* Scheduler include files. */
#include <stdlib.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "WPM.h"
#include "Custom Tasks/Current Position Store/currentposition.h"
#include "Custom Tasks/Error/ErrorMode.h"
#include "Custom Tasks/Display/globaldisplay.h"

// The number of positions the user can store and therefore the depth of the stack
#define SAVED_POSITIONS_MAX 50

/* Wait for 420ms before timing out while waiting for notifications
This has been chosen as the servo speed is 0.14seconds per 60 degress.
over 180 degrees this means a slowest time of 420 ms for the servo to move the whole distance. 
rounded up to 500 so that there is a pause and to account for tolerances. */
#define NOTIFY_WAIT_TIM_OUT ( TickType_t ) 420 


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
     xArmPosition_t arm_position;
    bool is_used;
};

/*-----------------------------------------------------------------------*/
/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vWPMTask, pvParameters );

/* The action functions */
void prvActionStop( struct xActionArgs args );
void prvActionSave( struct xActionArgs args );
void prvActionClear( struct xActionArgs args );
void prvActionReset( struct xActionArgs args );
void prvActionRun( struct xActionArgs args );

/*-----------------------------------------------------------------------*/
/* the queue which the task will receive from */
QueueHandle_t xWPMOutputQueue = NULL;

/* The stack of saved positions */
static struct stack_item_t pxStack[SAVED_POSITIONS_MAX];
int16_t iCurrentSize = SAVED_POSITIONS_MAX;

static const char* prvMSG_ON_SAVE = "Saved";
static const char* prvMSG_ON_RUN = "Running";
static const char* prvMSG_ON_CLR = "Cleared";
static const char* prvMSG_ON_STOP = "Stop";
static const char* prvMSG_ON_RST = "Reset";

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

//bool bNotifcationRxd = false;

struct xActionArgs xActionPckg;
xActionPckg.pxNextAction = &xNextAction;
xActionPckg.pxCurrentDirection = &xDirection;
xActionPckg.pusCurrentStackPosition = &usCurrentStackPosition;
xActionPckg.pusNextFreeStackPosition = &usNextFreeStackPosition;

    /* the meat of the task */
    for (;;)
    {
        //wait for the notification
        xTaskNotifyWait( ULONG_MAX,             /* Clear all on entry */
                         ULONG_MAX,             /* Reset the notification value to 0 on exit. */
                         &uNotificationValue,   /* Notified value pass out in ulNotifiedValue. */
                         NOTIFY_WAIT_TIM_OUT ); /* How long between the servo movements */

        // if rx'd
        if( uNotificationValue != 0 )
        {
        	switch( uNotificationValue )
        	{
        		case WPM_NOTIFICATION_STOP:
                    vSendToDisplayQueue( prvMSG_ON_STOP, strlen( prvMSG_ON_STOP ), wpmStop );
        			prvActionStop( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_SAVE:
                    vSendToDisplayQueue( prvMSG_ON_SAVE, strlen( prvMSG_ON_SAVE ), wpmSave );
        			prvActionSave( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_RESET:
                    vSendToDisplayQueue( prvMSG_ON_RST, strlen( prvMSG_ON_RST ), wpmReset );
        			prvActionReset( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_RUN:
                    vSendToDisplayQueue( prvMSG_ON_RUN, strlen( prvMSG_ON_RUN ), wpmRun );
        			xDirection = FORWARD;
        			prvActionRun( xActionPckg );
        			break;

        		case WPM_NOTIFICATION_CLEAR:
                    vSendToDisplayQueue( prvMSG_ON_CLR, strlen( prvMSG_ON_CLR ), wpmClear );
        			prvActionClear( xActionPckg );
        			break;

        		default:
        			/* unknown notifcation rx'd */
        			break;
        	}
            static char buff[DISPLAY_MAX_MSG_LEN];
            iConvertIntToString( SAVED_POSITIONS_MAX - usNextFreeStackPosition, buff );
            vSendToDisplayQueue( buff, strlen( buff ), wpmPointsRemaining );
        } 
        else if( xNextAction == RUN )
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
   // struct stack_item_t **ppxStack;
	xArmPosition_t xCurrentPosition = xGetCurrentPosition();

	// Check for re-assignment of stack size
	if( *( args.pusNextFreeStackPosition ) != iCurrentSize )
	{
        if( !pxStack[*( args.pusNextFreeStackPosition )].is_used )
        {
        int t = *( args.pusNextFreeStackPosition );
        /* save the arm position then move to the next high up item in the stack */
		pxStack[t].arm_position = xCurrentPosition;
        pxStack[t].is_used = true;
        *( args.pusCurrentStackPosition ) = *( args.pusNextFreeStackPosition );
        *( args.pusNextFreeStackPosition ) += 1;
        }
        else
        {
            // if it gets here then there is an error with the stack positioning
            // and something is used but the index pointers think otherwise
            vSetErrorConditon( "Stack positioning", strlen( "Stack positioning" ) );
        }
        
	}
	else
	{
		// Full stack!
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
        *( args.pusNextFreeStackPosition ) -= 1;
		pxStack[*( args.pusNextFreeStackPosition )].is_used = false;
        *( args.pusCurrentStackPosition ) = *( args.pusCurrentStackPosition ) > 0 ? *( args.pusCurrentStackPosition )-- : 0;
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

    // range check needs at least one item in the stach
	if( sNextStack >= 0 && sNextStack < iCurrentSize )
	{
        if( pxStack[sNextStack].is_used )  
        {
            *( args.pusCurrentStackPosition ) = ( uint8_t )sNextStack;
            
            if( pdFALSE == xQueueSend( xWPMOutputQueue, &pxStack[*( args.pusCurrentStackPosition )].arm_position, portMAX_DELAY ) )
            {
                vSetErrorConditon( "WPM Q Fail \r\n", strlen("WPM Q Fail \r\n") );   
            }
            else
            {
                *args.pxNextAction = RUN; 
                char buff[DISPLAY_MAX_MSG_LEN];
                iConvertIntToString( *( args.pusCurrentStackPosition ), buff );
                vSendToDisplayQueue( buff, strlen( buff ), wpmCurrentPoint );
            }
        }
		else 
        {
            // Not at the top of the max stack items but at the top of the currently allocated slots
            // Stop this loop
            *args.pxNextAction = NONE;
            vSendToDisplayQueue( "Complete", strlen( "Complete" ), wpmStop );
        }
        
	}
	else
	{
        // Stop in case of reaching start/end of stack
		*args.pxNextAction = NONE;
        vSendToDisplayQueue( "Complete", strlen( "Complete" ), wpmStop );
	}
}

/*-----------------------------------------------------------------------*/
/* init */
TaskHandle_t xStartWPMTask( int priority, xWPMParams_t *pxParams )
{
    xWPMOutputQueue = *( pxParams->pxServoInputQueue );
    iCurrentSize = SAVED_POSITIONS_MAX; // arbitrarily chosen
    int i;
    
    xArmPosition_t temp;
    temp.usBaseElevation = 0;
    temp.usBaseRotation = 0;
    temp.usElbow = 0;
    temp.usGrabber = 0;
    temp.usWristPitch = 0;
    temp.usWristRoll = 0;
    
    for( i = 0; i < SAVED_POSITIONS_MAX; i++)
    {
        pxStack[i].is_used = false;
        pxStack[i].arm_position = temp;
               
    }
    TaskHandle_t rc;
    //this stack size will need changing as it needs more room for the stack of positions
    xTaskCreate( vWPMTask, "WPM", configMINIMAL_STACK_SIZE + ( sizeof( struct stack_item_t) * SAVED_POSITIONS_MAX ), ( void* ) NULL , priority, ( TaskHandle_t* ) &rc );
    return rc;
}

