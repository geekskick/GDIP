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

#ifndef KEYPAD_H
#define KEYPAD_H

/* Each keypad button has an ASCII value associated with it in the following layout 
       |
    a b c d
    e f g h 
    i j k l 
    m n o p 
    */

    
#define KEYPAD_QUEUE_SIZE 10                /* the depth of the output queue, 10 is more than enough */
#define KEYPAD_TASK_PERIODICITYms 200       /* how often the whole keypad needs to be scanned for button press */
    
QueueHandle_t xStartKeypadTask( int priority, xComPortHandle com, TaskHandle_t *xTxTask );
void vSetDisplayTaskHandle( xTaskHandle xDisplayTask );
    
#endif

/* [] END OF FILE */
