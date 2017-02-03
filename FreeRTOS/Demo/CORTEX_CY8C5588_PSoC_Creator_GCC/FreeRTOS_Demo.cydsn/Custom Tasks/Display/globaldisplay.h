/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

#ifndef GLOB_DISP_H
#define GLOB_DISP_H
    
#include "serial.h"

#define DISPLAY_MAX_MSG_LEN 16	// A 16 x 2 lcd screen
#define DISPLAY_QUEUE_LEN	10  // 10 messages in the queue 

typedef struct displayParams
{
    QueueHandle_t *pxInputQueue;
 } xDisplayParams_t;

/* accessors */
QueueHandle_t xGetDisplayInputQueue( void );
xComPortHandle xGetDisplayComPortHandle( void );
xTaskHandle xGetDisplayTaskHandle( void );
void vSetDisplayComPortHandle( xComPortHandle xNewHandle );

/* usage */
void vSendToDisplayQueue( const char* sMessage, const size_t ulMessageLength );	
void vNotifyDisplayQueue( const uint32_t uNotificationValue ); 
void vWriteToComPort( const signed char*sMessage, const size_t ulMessageLength );

void vStartDisplayTask( int iPriority, xDisplayParams_t *pxParams );

//void vConvertIntToString( const int iInt, char*sOutputBuffer); //not implemented
    
#endif
/* [] END OF FILE */
