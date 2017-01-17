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

struct xDecoderParams_t
{
    QueueHandle_t xKeypadQueue;
};

QueueHandle_t xStartDecoderTask( int priority, struct xDecoderParams xInputParams );
    
#endif

/* [] END OF FILE */
