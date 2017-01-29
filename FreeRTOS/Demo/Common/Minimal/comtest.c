/*
    FreeRTOS V9.0.0rc2 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo program include files. */
#include "serial.h"
#include "comtest.h"
#include "partest.h"

//#include "currentposition.h"
#include "Custom Tasks/Current Position Store/currentposition.h"
#include "Custom Tasks/Display/globaldisplay.h"

#define comSTACK_SIZE				configMINIMAL_STACK_SIZE
#define comBUFFER_LEN				( 10 )

/* The receive task as described at the top of the file. */
static portTASK_FUNCTION_PROTO( vComRxTask, pvParameters );
static portTASK_FUNCTION_PROTO( vComTxTask, pvParameters );

/*-----------------------------------------------------------*/
static void vSerialPrint(const char8* cBuffer, const char8 cButton);
static void vScreenPrint(const char8* cBuffer, const char8 cButton);

/*-----------------------------------------------------------*/
static const char8* cBTN_MSG = "Button:";    /* boilerplate text for displaying */
static const char8* cPOS_MSG = "Servo:";
static uint8_t usBTN_MSG_LEN;
static uint8_t usPOS_MSG_LEN;
static xComPortHandle xCPHandle;
static xTaskHandle xTHandle;

/*-----------------------------------------------------------*/

void vAltStartComTestTasks( UBaseType_t uxPriority, uint32_t ulBaudRate )
{
    ( void )uxPriority; //stop warnings
    
	/* Initialise the com port then spawn the Rx and Tx tasks. */
    xCPHandle = xSerialPortInitMinimal( ulBaudRate, comBUFFER_LEN );
    //vSetDisplayComPortHandle( xCPHandle );
   
    /* create the tasks, the COMTx Task needs a larger stack a it causes a stack overflow */
    
    // commented out for debugging
	//xTaskCreate( vComRxTask, "COMRx", comSTACK_SIZE, ( void* ) &xParams.xRxdQueue,    uxPriority, ( TaskHandle_t * ) NULL );
    //xTaskCreate( vComTxTask, "COMTx", comSTACK_SIZE * 2, ( void* ) NULL, uxPriority, ( TaskHandle_t * ) &xTHandle );
    //vSetDisplayTaskHandle( xTHandle );
    
    /* in displaying writing to the comport you need to know the length of the string, it's const so calculate this only once. */
    usBTN_MSG_LEN = strlen( cBTN_MSG );
    usPOS_MSG_LEN = strlen( cPOS_MSG );
    
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vComRxTask, pvParameters )
{
signed char cByteRxed;      /* The input byte */
char buffer[comBUFFER_LEN] = { 0 };    /* a buffer to store the user input */
uint8_t bufferLoc = 0;      /* index of the next free location in the buffer */

	/* cast the params passed in as a pointer to a queue */
	QueueHandle_t xInputQueue = *( ( QueueHandle_t* ) pvParameters );

	for( ;; )
	{
		/* Block on the queue that contains received bytes until a byte is
		available. */
		xSerialGetChar( xCPHandle, &cByteRxed, portMAX_DELAY ); 
        
        /* turn the light on to show that it's in this part of the process */
        //vParTestToggleLED(0);
        
        /* if it's the end of what the user wants to send. or there is no more room then send to the queue */
        if( cByteRxed == '\r' || bufferLoc == comBUFFER_LEN - 1 ){
            
            /* first convert the recieved text into a number and send to the queue if it's valid */
            int iNumRxd = atoi( buffer );
            
            if( xInputQueue != NULL )
            {
                /* for now dont worry about a time out if the queue is full, dont think it'll ever get there */
                xQueueSend( xInputQueue, ( void * )&iNumRxd, 0 );
            }
            
            /* reset the buffer to 0s and point to the start of it */
            memset( buffer, 0, comBUFFER_LEN );
            bufferLoc = 0;
        }
        else
        {
            /* add to the buffer cause the user is still sending characters */
            buffer[bufferLoc++] = cByteRxed;   
        }    
	}
}

/*-----------------------------------------------------------*/
/* prints to serial port */
void vSerialPrint(const char8* cBuffer, const char8 cButton)
{

    /* clear the screen and reset to the top using the VT100 escape commands
    http://www.termsys.demon.co.uk/vtansi.htm */
    vSerialPutString( xCPHandle, ( const signed char* )"\033[2J", strlen( "\033[2J" ) );
    vSerialPutString( xCPHandle, ( const signed char* )"\033[0;0H", strlen( "\033[0;0H" ) );
           
    /*the display logic */
    vSerialPutString( xCPHandle, ( const signed char* )cBuffer, strlen( cBuffer ) );
    vSerialPutString( xCPHandle, ( const signed char* )"\n", 1 );
    vSerialPutString( xCPHandle, ( const signed char* )cBTN_MSG, usBTN_MSG_LEN );
    vSerialPutString( xCPHandle, ( const signed char* )&cButton , 1 );
}
/*-----------------------------------------------------------*/
/* prints to the LCD display */
/* THIS HAS AN INTERMITTANT FAULT; IN THE PRINTSTRING I THINK */
void vScreenPrint(const char8* cBuffer, const char8 cButton)
{
    /* not sure if the LCD API uses interrupts to test the ready flag, so 
    keep the interrupts enabled but stop the scheduler to do all the screen displaying at once 
    */
    portENTER_CRITICAL();
    
    LCD_DisplayOff();
    LCD_ClearDisplay();
    
    LCD_Position( 0u , 0u );
    LCD_PrintString( cBTN_MSG );
    LCD_Position( 0u, usBTN_MSG_LEN );
    LCD_PutChar( cButton );

    LCD_Position( 1u, 0u );
    LCD_PrintString( cPOS_MSG );
    LCD_Position( 1u, usPOS_MSG_LEN );
    LCD_PrintString( cBuffer );
    LCD_DisplayOn();

    portEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/
static portTASK_FUNCTION( vComTxTask, pvParameters )
{
( void ) pvParameters;                           /* stop warnings */
char8 buffer[comBUFFER_LEN] = { 0 };             /* a buffer to store the output in */
const TickType_t xFreq = 200;                    /* This is going to happen evert 200ms */
uint16_t     usCurrentPos = 0;                   /* will be parsed to make a string */
uint16_t     usPreviousPos = 0;                  /* for remembering the previous position */
TickType_t  xLastWakeTime = xTaskGetTickCount();/* init the tick count */
char8 cButton = 'X';                            /* when no button is pressed display this */
uint32_t ulBtn;                                 /* the returned notifcation value */

    /* init the screen */
    vScreenPrint( buffer, cButton );
    vScreenPrint( buffer, cButton );

	for( ;; )
    {
        
        /* only display things differently if there is a change or a button press detected, if there's no change then 
        don't bother displaying on screen or serial, The notification is set to 0 if after a read, and if nothing has 
        been sending a notification then it'll be 0 too.*/
        ulBtn = ulTaskNotifyTake( pdTRUE, ( TickType_t ) 0 );
        if( ulBtn != 0 )
        {
            /* the notification value returned is actually an ASCII character so explicitly cast it then refresh */
            cButton = ( char8 ) ulBtn;
            vScreenPrint( buffer, cButton );  
            vSerialPrint( buffer, cButton );

        }
        
        //also check the queue for debug messages
        
        /* The servo position is returned as a uint16_t, 
        so safely change this to a string before sending it */
        usCurrentPos = usGetCurrentPosition();
        
        if( usPreviousPos != usCurrentPos )
        {
            
            /* zero the buffer so that no extra numbers remain */
            memset( buffer, 0, comBUFFER_LEN );
            snprintf( buffer, comBUFFER_LEN, "%d", usCurrentPos );
            
            vSerialPrint( buffer, cButton );
            vScreenPrint( buffer, cButton );
            
            /* this is now the most recent number */
            usPreviousPos = usCurrentPos;
           
        }
        
        /* delay */
        vTaskDelayUntil( &xLastWakeTime, xFreq );
	}
}