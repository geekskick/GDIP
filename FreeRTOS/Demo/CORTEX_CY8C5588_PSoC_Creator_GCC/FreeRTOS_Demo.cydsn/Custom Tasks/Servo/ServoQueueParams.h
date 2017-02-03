/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

#ifndef SERVOQUEUE_H
#define SERVOQUEUE_H
  
/* you can only add or substract from a servo value */
typedef enum xServoDirection{ ADD, SUB } xServoDirection_t;
typedef enum xServoNumber{ BaseRotation = 0, BaseElevation, Elbow, WristRoll, WristPitch, Grabber, END } xServoNumber_t;

/* this struct is sent from the decoder task to the servo task */
typedef struct xServoQueueParams
{
    xServoNumber_t    xServo;        /* which servo im talking about */
    xServoDirection_t xDirection;    /* add or substract */
} xServoQueueParams_t;

typedef struct xServoInputQueues
{
    QueueHandle_t *pxFromWPM;
    QueueHandle_t *pxFromKeypad;
} xServoInputQueues_t;

#endif

/* [] END OF FILE */
