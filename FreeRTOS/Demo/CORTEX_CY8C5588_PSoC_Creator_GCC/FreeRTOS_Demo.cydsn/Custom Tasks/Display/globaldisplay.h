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

#ifndef GLOB_DISP_H
#define GLOB_DISP_H
    
    /*
This functionality needs more thought - any task should be able to write to the 
display a message or send a notification. If theyr just get the queue and use it, there might 
be a race(?) condition if two accesing it at the same time. This module should do that writing
so that only one task can do it at a time.

A queue will be used for displaying to the screen, however this module will also provide conversion functions
for the data types into a char* buffer. A task may use it by doing:

vConvertInttoString( servoValue, sBuff );
vSendToDisplayQueue( sBuff, strlen( sBuff ) );

    */

#define DISPLAY_MAX_MSG_LEN 16	// A 16 x 2 lcd screen
#define DISPLAY_QUEUE_LEN	10  // 10 messages in the queue 

// might not need these functions after all
QueueHandle_t xGetDisplayInputQueue( void );
xComPortHandle xGetDisplayComPortHandle( void );
xTaskHandle xGetDisplayTaskHandle( void );

void vSetDisplayTaskHandle( xTaskHandle xNewHandle );
void vSetDisplayInputQueue( QueueHandle_t xNewQueue );
void vSetDisplayComPortHandle( xComPortHandle xNewHandle );

void vSendToDisplayQueue( const char* sMessage, const size_t ulMessageLength );	// not implemented
void vNotifyDisplayQueue( const uint32_t uNotificationValue ); // not implemented
void vWriteToComPort( const char*sMessage, const size_t ulMessageLength ); //not implememented, maybe not needed?

void vConvertIntToString( const int iInt, char*sOutputBuffer); //not implemented
    
#endif
/* [] END OF FILE */
