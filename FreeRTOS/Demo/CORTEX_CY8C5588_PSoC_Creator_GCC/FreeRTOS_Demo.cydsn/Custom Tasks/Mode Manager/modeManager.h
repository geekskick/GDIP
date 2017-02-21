/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 6th Feb 2017
Changes Made   : Initial Issue
*****************************************/
    
#ifndef MODE_MGR_H
#define MODE_MGR_H
    
#define MODE_SUBSCRIBERS_MAX 5
    
typedef enum xMode{
    INIT = 0, MANUAL = 1, TRAINING = 2, AUTO = 3
}xMode_t;

void vSubscribeToModeChange( void ( *fn )( xMode_t ) );
void vModeChange( void );
    
#endif // MODE_MGR_H