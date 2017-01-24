/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef SERVO_H
#define SERVO_H
    
#include "servoqueueparams.h"
    
/* depth of the queue is ten bytes */
#define SERVO_QUEUE_SIZE 10

void vStartServoTasks( int priority, xServoInputQueues_t *pxOutput )
uint16_t usGetMidPoint( void );

#endif

/* [] END OF FILE */
