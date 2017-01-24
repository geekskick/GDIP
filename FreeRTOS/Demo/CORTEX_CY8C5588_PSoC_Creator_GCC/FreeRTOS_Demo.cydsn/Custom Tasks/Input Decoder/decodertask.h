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
    
#define DECODER_INPUT_QUEUE_SIZE 10

typedef struct xDecoderParams
{
    QueueHandle_t *pxDecoderOutputQueue;
}xDecoderParams_t;

QueueHandle_t xStartDecoderTask( int priority, xDecoderParams_t *pxInputParams );
    
#endif

/* [] END OF FILE */
