#include "FreeRTOS.h"
#include "task.h"
#include "ErrorMode.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


/*---------------------------------------------------------------------------*/
char *sErrorMessage[ERROR_MSG_MAX_LEN] = { 0, };
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
    unsigned int i = 0;
    
    //copy the string
    while( i < ERROR_MSG_MAX_LEN && i < ulMsgLen )
    {
        sErrorMessage[i] = sMsg[i];
        i += 1;
    }
    
    bError = true;
    taskEXIT_CRITICAL();
     
};

/*---------------------------------------------------------------------------*/
void vErrorConditionHook( void )
{
    for(;;)
    {
        
    }
}