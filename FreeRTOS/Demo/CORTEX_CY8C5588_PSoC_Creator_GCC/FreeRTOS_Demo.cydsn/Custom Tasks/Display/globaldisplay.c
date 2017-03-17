/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

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
#include "../Custom Tasks/Mode Manager/modeManager.h"

/*-----------------------------------------------------------------------------*/
/* the type of the thing to be get/set */
typedef enum { ComPort, Task, Queue } xTypeToGet_t;

/*-----------------------------------------------------------------------------*/
/* the stored handles */
static QueueHandle_t xDispQueue;    /* Queue to the diplay task */
static xComPortHandle xComPort;     /* the UART handle */
static xTaskHandle xDispTask;        /* the Display task handle */
static xSemaphoreHandle xGatekeeper = NULL;     /* mutex */
static bool xInitialised = false;               /* if the mutex is created this is true */

/*-----------------------------------------------------------------------------*/
/* internal functions and a point to them mean I wont have to repeat myself 
    when writing code to access the mutex etc
*/
void prvGenericSet( xTypeToGet_t xType, void *in );
void prvGenericGet( xTypeToGet_t xType, void *in );
void prvGenericBase( void ( *GFunct ) ( xTypeToGet_t xType, void *pcThingToGetSet ), xTypeToGet_t xType, void* pxArg );
static portTASK_FUNCTION_PROTO( vDisplayTask, pvParameters );
void prvDisplayOnModeChange( xMode_t  xNewMode);

char topLine[16], bottomLine[16];
const uint8_t prviCurrentPoint = 14;
const uint8_t prviPointsRemaining = 14;

/*-----------------------------------------------------------------------------*/
/* displays on the screen */
static portTASK_FUNCTION( vDisplayTask, pvParamaters )
{
( void ) pvParamaters; // stop warnings
xDisplayQueueParams xInput;
bool update = false;
int i = 0;
    memset( topLine, 0x20, DISPLAY_MAX_MSG_LEN );
    memset( bottomLine, 0x20, DISPLAY_MAX_MSG_LEN );

    memset( topLine, ' ', DISPLAY_MAX_MSG_LEN );
    memset(bottomLine, ' ', DISPLAY_MAX_MSG_LEN );

    /* the meat of the task goes here */
    for(;;)
    {
       
        /* need to set the buffer to be cleared in order to have a 
        correct write to the screen */
        memset( xInput.msg, 0x20, DISPLAY_MAX_MSG_LEN );
        
        if( pdTRUE == xQueueReceive( xDispQueue, &xInput, portMAX_DELAY ) )
        {
            
            update = false;
            
            switch( xInput.type )
            {

            case wpmClear:
            case wpmSave:
            case mode:
                memset( bottomLine, ' ', DISPLAY_MAX_MSG_LEN );
                strncpy( bottomLine, xInput.msg, xInput.iMsgLen );
                update = true;
                break;
                
            case wpmPointsRemaining:
                memset(bottomLine + prviPointsRemaining, ' ', DISPLAY_MAX_MSG_LEN - prviPointsRemaining );
                strncpy( bottomLine + prviPointsRemaining, xInput.msg, xInput.iMsgLen );
                update = true;
                break;
                
            case wpmCurrentPoint:
                memset(topLine + prviCurrentPoint, ' ', DISPLAY_MAX_MSG_LEN - prviCurrentPoint );
                strncpy( topLine + prviCurrentPoint, xInput.msg, xInput.iMsgLen );
                update = true;
                break;
                
            case wpmReset:
            case wpmRun:
            case wpmStop:
                memset( topLine, ' ', prviCurrentPoint );
                strncpy( topLine, xInput.msg, xInput.iMsgLen );
           
                update = true;
                break;
                
            default:
                break;
            }
            
            // The following section of writing to the screen requires critical timing between the screen and the PCB
            if( update )
            {
                vTaskSuspendAll();
                LCD_ClearDisplay();
                
                LCD_Position( 0, 0 );
                
                int i = 0;
                for( i = 0; i < DISPLAY_MAX_MSG_LEN; i++ )
                { 
                    LCD_PutChar( topLine[i] );
                }
                
                LCD_Position( 1, 0 );
                
                for( i = 0; i < DISPLAY_MAX_MSG_LEN; i++ )
                { 
                    LCD_PutChar( bottomLine[i] );
                }
                
                xTaskResumeAll();
            }
        }
    }
}

/*-----------------------------------------------------------------------------*/
void prvDisplayOnModeChange( xMode_t  xNewMode)
{
static char buff[DISPLAY_MAX_MSG_LEN];
bool bSend = false;

    memset( buff, 0x00, DISPLAY_MAX_MSG_LEN );

    switch( xNewMode )
    {
    case AUTO:
        strcpy( buff, "Auto" );
        bSend = true;
        break;
    case MANUAL:
        strcpy( buff, "Manual" );
        bSend = true;
        break;
    case TRAINING:
        strcpy( buff, "Trg" );
        bSend = true;
        break;
    case INIT:
        strcpy( buff, "Init" );
        bSend = true;
        break;
        
     default:
        
        break;
    }
    
    if ( bSend )
    {
        vSendToDisplayQueue( buff, strlen(buff), mode );
    }
}
/*-----------------------------------------------------------------------------*/
void prvGenericSet( xTypeToGet_t xType, void *in )
{
	switch( xType )
	{
		case ComPort: 	xComPort = *( xComPortHandle* )in; 	break;
		case Queue: 	xDispQueue = *( QueueHandle_t* )in; break;
		case Task: 		xDispTask = *( xTaskHandle* )in; 	break;
		default: break;
	}
}

/*-----------------------------------------------------------------------------*/
void prvGenericGet( xTypeToGet_t xType, void* out )
{
	switch( xType )
	{
		case ComPort: 	*( xComPortHandle* )out = &xComPort; 	break;
		case Queue: 	*( QueueHandle_t* )out = &xDispQueue; 	break;
		case Task: 		*( xTaskHandle* )out = &xDispTask; 		break;
		default: break;
	}

}

/*-----------------------------------------------------------------------------*/
/* Does the mutex lock/unlocking before calling the accessor function  */
void prvGenericBase( void ( *GFunct ) ( xTypeToGet_t xType, void *pcThingToGetSet ), xTypeToGet_t xType, void* pxArg )
{

    /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
    */
    if( xInitialised )
    {
        if( pdTRUE == xSemaphoreTake( xGatekeeper, ( TickType_t ) 20 ) ) // 20 ms block time arbitrarily picked for now
        {
            GFunct( xType, pxArg );
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
            prvGenericBase( GFunct, xType, pxArg );
        }
    }   
}

/*------------------------------------------------------------------*/
xTaskHandle xGetDisplayTaskHandle( void )
{
    xTaskHandle rc;
    prvGenericBase( &prvGenericGet, Task, ( void* )&rc );
    return rc;
}

/*------------------------------------------------------------------*/
QueueHandle_t xGetDisplayInputQueue( void )
{
    QueueHandle_t rc;
    prvGenericBase( &prvGenericGet, Queue, ( void* )&rc );
    return rc;
}

/*------------------------------------------------------------------*/
xComPortHandle xGetDisplayComPortHandle( void )
{
    xComPortHandle rc;
    prvGenericBase( &prvGenericGet, ComPort, ( void* )&rc );
    return rc;
}

/*------------------------------------------------------------------*/
void vSetDisplayTaskHandle( xTaskHandle xNewHandle )
{
	prvGenericBase( &prvGenericSet, Task, ( void* )&xNewHandle );
}

/*------------------------------------------------------------------*/
void vSetDisplayInputQueue( QueueHandle_t xNewQueue )
{
	prvGenericBase( &prvGenericSet, Queue, ( void* )&xNewQueue );
}

/*------------------------------------------------------------------*/
void vSetDisplayComPortHandle( xComPortHandle xNewHandle )
{
	prvGenericBase( &prvGenericSet, ComPort, ( void* )&xNewHandle );
    vSerialPutString( xComPort, "Com port handle stored\r\n", strlen("Com port handle stored\r\n") );
}

/*------------------------------------------------------------------*/
/* Sends to the UART module a string */
void vWriteToComPort( const signed char *sMessage, const size_t ulMessageLength )
{
    /* The ComPortHandle argument isn't actually used, so no need to send the actual handle in */
    /* The serial put char function that this goes to enters taskCritical so no need to do that here */
    vSerialPutString( xComPort, sMessage, ulMessageLength );
}
    
/*------------------------------------------------------------------*/
/* The queue location might be used by different tasks at different times, 
 so use the mutex to lock access to it to prevent weird stuff being displayed */
void vSendToDisplayQueue( const char* sMessage, const size_t ulMessageLength, const xDisplayMsg_t xType )
{
static xDisplayQueueParams temp;
    /* If the mutex hasn't been initialised then don't allow access to the resource, 
     * make an attempt to create the mutex, and if it's created call the function again. 
     */
    if( xInitialised )
    {
        strcpy( temp.msg, sMessage );
        temp.iMsgLen = ulMessageLength;
        temp.type = xType;
        if( pdTRUE == xSemaphoreTake( xGatekeeper, portMAX_DELAY ) ) 
        {
            if( xDispQueue != 0 )
            {
                if( pdFALSE == xQueueSend( xDispQueue, ( void* )&temp, portMAX_DELAY ) )
                {
                    /* there has been an error in sending to the queue */
                }
            }
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
            vSendToDisplayQueue( sMessage, ulMessageLength, xType );
        }
    } 
    
}

/*------------------------------------------------------------------*/
int iConvertIntToString( int iNum, char* dst )
{
    memset( dst, 0, DISPLAY_MAX_MSG_LEN );
    snprintf( dst, DISPLAY_MAX_MSG_LEN, "%d", iNum );
    return strlen( dst );
}
/*------------------------------------------------------------------*/
void vStartDisplayTask( int iPriority, xDisplayParams_t *pxParams )
{
    xDispQueue = xQueueCreate( DISPLAY_QUEUE_LEN, sizeof( xDisplayQueueParams ) );
    pxParams->pxInputQueue = &xDispQueue;
    vSubscribeToModeChange( &prvDisplayOnModeChange );
    
    xTaskCreate( vDisplayTask, "Display", configMINIMAL_STACK_SIZE * 4, NULL, iPriority, ( TaskHandle_t* ) &xDispTask );
}

/*------------------------------------------------------------------*/
/* [] END OF FILE */
