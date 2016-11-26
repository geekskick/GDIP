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

#ifndef SERVOQUEUE_H
#define SERVOQUEUE_H
  
enum xServoDirection{ ADD, SUB };
struct xServoQueueParams
{
    uint8_t usServoNumber;
    enum xServoDirection xDirection;    
};
    
#endif

/* [] END OF FILE */
