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
    
/* depth of the queue is ten bytes */
#define SERVO_QUEUE_SIZE 10

QueueHandle_t xStartServoTasks( int priority, QueueHandle_t xInputQueue );
uint16_t usGetMidPoint( void );

#endif

/* [] END OF FILE */
