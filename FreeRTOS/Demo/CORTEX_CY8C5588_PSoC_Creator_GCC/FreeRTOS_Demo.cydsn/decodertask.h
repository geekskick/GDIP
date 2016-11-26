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

#ifndef DECODERTASK_H
#define DECODERTASK_H

struct xDecoderParams
{
    QueueHandle_t xKeypadQueue;
    char8         *cpDisplayTaskName;
    char8         *cpWaypointManagerTaskName;
};

QueueHandle_t xStartDecoderTask( int priority, struct xDecoderParams xParams );
    
#endif

/* [] END OF FILE */
