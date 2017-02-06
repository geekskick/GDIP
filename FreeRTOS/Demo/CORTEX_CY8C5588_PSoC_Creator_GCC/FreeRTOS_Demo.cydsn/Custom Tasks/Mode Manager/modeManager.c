/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 6th Feb 2017
Changes Made   : Initial Issue
*****************************************/

#include "modeManager.h"
#include <stdint.h>

//----------------------------
xMode_t xCurrentMode = INIT;
void ( *callbacks[MODE_SUBSCRIBERS_MAX] )( xMode_t );
uint8_t iNumberSubscribers = 0;

//----------------------------
void prvNotify( void );

//----------------------------
void vModeChange( void )
{
    if( xCurrentMode == AUTO ) 
    {
        xCurrentMode = MANUAL;   
    }
    else
    {
        xCurrentMode++;   
    }
    
    prvNotify();
}

//----------------------------
void prvNotify( void )
{
    uint8_t i;
    for( i = 0; i <= iNumberSubscribers; i++ )
    {
        callbacks[i]( xCurrentMode );
        
    }   
}
//----------------------------
void vSubscribeToModeChange( void ( *fn )( xMode_t ) )
{
    callbacks[iNumberSubscribers++] = fn;
}