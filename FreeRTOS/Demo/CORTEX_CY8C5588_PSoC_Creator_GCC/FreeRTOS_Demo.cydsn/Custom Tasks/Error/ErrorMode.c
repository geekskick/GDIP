/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "ErrorMode.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "Custom Tasks/Current Position Store/currentposition.h"
#include "Custom Tasks/Servo/servo.h"


/*---------------------------------------------------------------------------*/
char sErrorMessage[ERROR_MSG_MAX_LEN] = { 0, };
bool bError = false;

/*---------------------------------------------------------------------------*/
/* no need for a mutex - this is called from the task scheduler in context switching */
void gdipCheckForErrorCondition( void )
{
    if( bError )
    {
        vErrorConditionHook();   
    }
};
	
/*---------------------------------------------------------------------------*/
/* this will be called by user tasks */
/* no need for a mutex - just prevent a context switch while the string is being copied */
void vSetErrorConditon( const char* sMsg, const size_t ulMsgLen )
{
    taskENTER_CRITICAL();

    strcpy( sErrorMessage, sMsg );

    taskEXIT_CRITICAL();
     
};

/*---------------------------------------------------------------------------*/
void vErrorConditionHook( void )
{
    taskDISABLE_INTERRUPTS();
    for(;;)
    {
        UART_PutString( sErrorMessage );
        const uint16_t usMidPoint = usGetMidPoint();

    baseRotationPWM_Start();
    baseElevationPWM_Start();
    elbowPWM_Start();
    wristPitchPWM_Start();
    wristRollPWM_Start();
    grabberPWM_Start();
   
    /* init the servo to middle */
    baseRotationPWM_WriteCompare( usMidPoint );
    baseElevationPWM_WriteCompare( usMidPoint );
    elbowPWM_WriteCompare( usMidPoint );
    wristPitchPWM_WriteCompare( usMidPoint );
    wristRollPWM_WriteCompare( usMidPoint );
    grabberPWM_WriteCompare( usMidPoint );

    /* the arms is in the mid point position - so put it in the storage area */
     xArmPosition_t xTempPosition = {
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint
    };
    
    }
}