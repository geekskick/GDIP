/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 6th Feb 2017
Changes Made   : Initial Issue
*****************************************
Change ID      : NA
Version        : 2
Date           : 12th Feb 2017
Changes Made   : 
    LEDs indicate mode
    00 = Man
    10 = Trg
    01 = Auto
    11 = Error
*****************************************/

#include "modeManager.h"
#include <stdint.h>
#include "portmacro.h"

//----------------------------
xMode_t xCurrentMode = INIT;
void ( *callbacks[MODE_SUBSCRIBERS_MAX] )( xMode_t );
uint8_t iNumberSubscribers = 0;

//----------------------------
void prvNotify( void );

//----------------------------
void vModeChange( void )
{
    portENTER_CRITICAL();
    errorLED_Write( 0 );
    
    if( xCurrentMode == AUTO ) 
    {
        xCurrentMode = MANUAL;   
    }
    else
    {
        xCurrentMode++;   
    }
    
    prvNotify();
    portEXIT_CRITICAL();
}

//----------------------------
void prvNotify( void )
{
    uint8_t i;
    for( i = 0; i < iNumberSubscribers; i++ )
    {
        callbacks[i]( xCurrentMode );
        
    }  
}
//----------------------------
void vSubscribeToModeChange( void ( *fn )( xMode_t ) )
{
    callbacks[iNumberSubscribers++] = fn;
}