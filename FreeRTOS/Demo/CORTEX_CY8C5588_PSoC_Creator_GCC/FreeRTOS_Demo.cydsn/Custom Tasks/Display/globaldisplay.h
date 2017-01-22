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

#ifndef GLOB_DISP_H
#define GLOB_DISP_H
    
QueueHandle_t xGetDisplayInputQueue( void );
xComPortHandle xGetDisplayComPortHandle( void );
xTaskHandle xGetDisplayTaskHandle( void );

void vSetDisplayTaskHandle( xTaskHandle xNewHandle );
void vSetDisplayInputQueue( QueueHandle_t xNewQueue );
void vSetDisplayComPortHandle( xComPortHandle xNewHandle );

    
    
#endif
/* [] END OF FILE */
