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
enum xServoDirection_t{ ADD, SUB };
enum xServoNumber_t{ BaseRotation = 0, BaseElevation, Elbow, WristRoll, WristPitch, Grabber, END };

/* this struct is sent from the decoder task to the servo task */
struct xServoQueueParams
{
    enum xServoNumber_t    xServo;        /* which servo im talking about */
    enum xServoDirection_t xDirection;    /* add or substract */
};
    
#endif

/* [] END OF FILE */
