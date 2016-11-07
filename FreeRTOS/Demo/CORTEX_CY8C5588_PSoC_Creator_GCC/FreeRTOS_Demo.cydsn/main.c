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

#include <device.h>

/* RTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Common Demo includes. */
#include "serial.h"
#include "comtest.h"

#include "partest.h"
/*---------------------------------------------------------------------------*/
/* The number of nano seconds between each processor clock. */
#define mainNS_PER_CLOCK ( ( unsigned long ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Task priorities. */
#define mainCOM_TEST_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

/* A struct to pass paramters to the servo task */
struct servoArgs{
    xComPortHandle serialPort;
    QueueHandle_t  inputQueue;
};

/* the queue i only want to be visible in this file so make it static */
static QueueHandle_t servoQueue = NULL;

/* forward declare it cause im a good boy */
static portTASK_FUNCTION_PROTO( vServoTask, pvParameters );
/*---------------------------------------------------------------------------*/

/*
 * Installs the RTOS interrupt handlers and starts the peripherals.
 */
static void prvHardwareSetup( void );

/*-----------------------------------------------------------------------*/
static portTASK_FUNCTION( vServoTask, pvParamaters )
{
    /* get out the paramters */
    struct servoArgs *args = ( ( struct servoArgs* ) pvParamaters );
    uint8_t inputValue = 0;
    const uint8_t SERVO_MIN = 0x00, 
                  SERVO_MAX = 0xFF;
    
    // the meat of the task
    for (;;)
    {
        /* block forever to get the value from the queue, for some reason using the servoQueue passed in blocks forever in the rc function as well */
        if( pdTRUE == xQueueReceive( args->inputQueue, &inputValue, portMAX_DELAY ) )
        {
            /* The LED should tun off immediately */
            vParTestToggleLED(0);
            
            /* when using HW disable interurpts */
            taskENTER_CRITICAL();
            
            /* for now this test is pointless, however when i actually know these limits
            it's easy to change the values in the variables. Set the Duty Cycle to within limits, and not over the set period */
            if( ( inputValue <= SERVO_MAX ) &&
                ( inputValue > SERVO_MIN ) &&
                ( inputValue < servoPWM_ReadPeriod() ) 
            )
            { 
                servoPWM_WriteCompare( inputValue ); 
            }
            
            /* re-enable interrupts */
            taskEXIT_CRITICAL();
        }
        
    }
    
}
/*---------------------------------------------------------------------------*/
int main( void )
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	prvHardwareSetup();

    /* init the queue between the uart in and the servo task with enough room for 100 bytes of data to be sent.
    it wont ever be that much but I need to make a start somewhere. */
    servoQueue = xQueueCreate( 100, sizeof(uint8_t) );
    
    /* start the comms with a baudrate of 9600 and the address of the servo queue which it'll be writing to
    and return the comport handler into the uart location
    */
	xComPortHandle uart = NULL; 
    vAltStartComTestTasks( mainCOM_TEST_TASK_PRIORITY, 9600, &servoQueue, &uart );
    
    /* The struct for passing to the task need to be static  */
    static struct servoArgs a;
    a.inputQueue = servoQueue;
    a.serialPort = uart;
    
    /* finally create the servo task */
    xTaskCreate( vServoTask, "Servo", configMINIMAL_STACK_SIZE, ( void* ) &a, mainCOM_TEST_TASK_PRIORITY - 1, ( TaskHandle_t *) NULL);

	/* Will only get here if there was insufficient memory to create the idle
    task.  The idle task is created within vTaskStartScheduler(). */
	vTaskStartScheduler();

	/* Should never reach here as the kernel will now be running.  If
	vTaskStartScheduler() does return then it is very likely that there was
	insufficient (FreeRTOS) heap space available to create all the tasks,
	including the idle task that is created within vTaskStartScheduler() itself. */
	for( ;; );
}
/*---------------------------------------------------------------------------*/

void prvHardwareSetup( void )
{
/* Port layer functions that need to be copied into the vector table. */
extern void xPortPendSVHandler( void );
extern void xPortSysTickHandler( void );
extern void vPortSVCHandler( void );
extern cyisraddress CyRamVectors[];

	/* Install the OS Interrupt Handlers. */
	CyRamVectors[ 11 ] = ( cyisraddress ) vPortSVCHandler;
	CyRamVectors[ 14 ] = ( cyisraddress ) xPortPendSVHandler;
	CyRamVectors[ 15 ] = ( cyisraddress ) xPortSysTickHandler;

	/* Start-up the peripherals. */

	/* Start the UART. */
	UART_Start();
    
    /* Start the pwm, as it comprises of a clock and the PWM module both need doing */
    pwmClock_Start();
    servoPWM_Start();
    
    /* Start the LED as it's max brightness, just so I can see it's working */
    uint8_t maxP = servoPWM_ReadPeriod();
    servoPWM_WriteCompare(maxP);
    
    /* also the same for the built in LED */
    builtInLED_Write(1);

}
/*---------------------------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*---------------------------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* The heap space has been execeeded. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*---------------------------------------------------------------------------*/
