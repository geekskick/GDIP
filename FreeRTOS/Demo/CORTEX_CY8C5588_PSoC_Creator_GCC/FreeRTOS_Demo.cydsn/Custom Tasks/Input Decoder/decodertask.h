/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

#ifndef DECODERTASK_H
#define DECODERTASK_H
    
#define DECODER_INPUT_QUEUE_SIZE 10

typedef struct xDecoderParams
{
    QueueHandle_t *pxDecoderOutputQueue;
    TaskHandle_t xWPMTaskHandle;
    TaskHandle_t *pxKeypadHandle;
}xDecoderParams_t;

QueueHandle_t xStartDecoderTask( int priority, xDecoderParams_t *pxInputParams );
    
#endif

/* [] END OF FILE */
