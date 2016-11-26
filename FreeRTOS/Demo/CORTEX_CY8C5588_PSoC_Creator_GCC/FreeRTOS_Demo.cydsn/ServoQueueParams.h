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
  
/* you can only add or substract from a servo value */
enum xServoDirection{ ADD, SUB };

/* this struct is sent from the decoder task to the servo task */
struct xServoQueueParams
{
    uint8_t usServoNumber;              /* which servo im talking about */
    enum xServoDirection xDirection;    /* add or substract */
};
    
#endif

/* [] END OF FILE */
