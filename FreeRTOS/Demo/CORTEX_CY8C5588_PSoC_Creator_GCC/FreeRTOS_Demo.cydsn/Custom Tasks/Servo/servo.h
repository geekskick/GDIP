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
    Servo Queue size upped to 100
    to account for quicker keypad task
*****************************************/

#ifndef SERVO_H
#define SERVO_H
    
#include "servoqueueparams.h"
    
#define SERVO_QUEUE_SIZE 100

void vStartServoTasks( int priority, xServoInputQueues_t *pxOutput );
uint16_t usGetMidPoint( void );

#endif

/* [] END OF FILE */
