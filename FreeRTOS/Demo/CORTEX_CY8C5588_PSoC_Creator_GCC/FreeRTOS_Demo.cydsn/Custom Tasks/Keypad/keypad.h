/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

#ifndef KEYPAD_H
#define KEYPAD_H

/* Each keypad button has an ASCII value associated with it in the following layout 
       |
    a b c d
    e f g h 
    i j k l 
    m n o p 
    */

    
#define KEYPAD_QUEUE_SIZE           10         /* the depth of the output queue, 10 is more than enough */

    
typedef struct xKeypadParams
{
    QueueHandle_t *pxOutputQueue;
} xKeypadParams_t;

QueueHandle_t xStartKeypadTask( int priority, xKeypadParams_t *pxParams );
void vSetDisplayTaskHandle( TaskHandle_t xDisplayTask );
    
#endif

/* [] END OF FILE */
