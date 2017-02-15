/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 2
Date           : 10th Feb 2017
Changes Made   : 
    Length of queue changed to account for 
    quicker keypad task
*****************************************/



#ifndef GLOB_DISP_H
#define GLOB_DISP_H
    
#include "serial.h"

#define DISPLAY_MAX_MSG_LEN 16	// A 16 x 2 lcd screen
#define DISPLAY_QUEUE_LEN	100 // 100 messages in the queue 

typedef struct displayParams
{
    QueueHandle_t *pxInputQueue;
 } xDisplayParams_t;

typedef enum
{   baseElevation, baseRotation, elbow, wristPitch, wristRoll, grabber,  // servo position updates
    mode, // mode change
    wpmPointsRemaining,  // waypoint info
    btnPress, btnAccept, btnReject, // keypad feedback
    error 
} xDisplayMsg_t;

typedef struct displayQueueParams
{
    char msg[DISPLAY_MAX_MSG_LEN];
    int iMsgLen;
    xDisplayMsg_t type;
    
} xDisplayQueueParams;

/* accessors */
QueueHandle_t xGetDisplayInputQueue( void );
xComPortHandle xGetDisplayComPortHandle( void );
xTaskHandle xGetDisplayTaskHandle( void );
void vSetDisplayComPortHandle( xComPortHandle xNewHandle );

/* usage */
void vSendToDisplayQueue( const char* sMessage, const size_t ulMessageLength, const xDisplayMsg_t xType );	
void vNotifyDisplayQueue( const uint32_t uNotificationValue ); 
void vWriteToComPort( const signed char*sMessage, const size_t ulMessageLength );

void vStartDisplayTask( int iPriority, xDisplayParams_t *pxParams );
int iConvertIntToString( int iNum, char* dst );

//void vConvertIntToString( const int iInt, char*sOutputBuffer); //not implemented
    
#endif
/* [] END OF FILE */
