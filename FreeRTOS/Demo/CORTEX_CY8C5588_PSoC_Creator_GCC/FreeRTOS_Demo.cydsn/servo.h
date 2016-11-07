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
    
#include "Serial.h" /* included for the xComPortHandle"  */
    

/* A struct to pass paramters to the servo task */
struct servoArgs{
    xComPortHandle serialPort;
    QueueHandle_t  inputQueue;
};

void vStartServoTasks(void *pvParams, int priority);

#endif

/* [] END OF FILE */
