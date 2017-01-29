
#ifndef ERROR_MODE_H
#define ERROR_MODE_H
    
#define ERROR_MSG_MAX_LEN 20

/* this will be called from the context switching code */
void gdipCheckForErrorCondition( void );

/* this will be called by user tasks */
void vSetErrorConditon( const char* sMsg, const size_t ulMsgLen );
	
/* this will happen if there is an error condition */
void vErrorConditionHook( void );

#endif //Error_mode_h