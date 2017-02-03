/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

#ifndef SERVO_H
#define SERVO_H
    
#include "servoqueueparams.h"
    
/* depth of the queue is ten bytes */
#define SERVO_QUEUE_SIZE 10

void vStartServoTasks( int priority, xServoInputQueues_t *pxOutput );
uint16_t usGetMidPoint( void );

#endif

/* [] END OF FILE */
