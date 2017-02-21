/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 2
Date           : 12th Feb 2017
Changes Made   : 
    Error mode leds turn on
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
    
    // Display message
    UART_PutString( sErrorMessage );
    LCD_ClearDisplay();
    LCD_Position( 0, 0 );
    LCD_PrintString( "Error: " );
    LCD_Position( 1, 0 );
    LCD_PrintString( sErrorMessage );
    
    // Handle LED
    autoModeLED_Write( 1 );
    trgModeLED_Write( 1 );
    const uint16_t usMidPoint = usGetMidPoint();

    // Incase the PWM has stopped in a task
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
    
    for(;;){}
}