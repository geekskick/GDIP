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

/* GDIP Includes */
#include "Custom Tasks/Servo/servo.h"
#include "Custom Tasks/Servo/ServoQueueParams.h"
#include "Custom Tasks/Keypad/keypad.h"
#include "Custom Tasks/Current Position Store/currentposition.h"
#include "Custom Tasks/Input Decoder/decodertask.h"
#include "Custom Tasks/WPM/WPM.h"

/*---------------------------------------------------------------------------*/
/* The number of nano seconds between each processor clock. */
#define mainNS_PER_CLOCK ( ( unsigned long ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* 
 * Task priorities. These are assigned as a 'suck it and see' for now - static 
 * analysis of a task's worst case execution time is very difficult, and 
 * as this isn't intended to be a truly HARD RTOS then it's not THAT important.
 * Where a higher priority value means a higher priority task.
 */
#define mainDISPLAY_TEST_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainKEYPAD_TASK_PRIORITY                ( tskIDLE_PRIORITY + 2 )
#define mainDECODER_TASK_PRIORITY               ( tskIDLE_PRIORITY + 3 )
#define mainWPM_TASK_PRIORITY                   ( tskIDLE_PRIORITY + 4 )
#define mainSERVO_TASK_PRIORITY                 ( tskIDLE_PRIORITY + 5 )

/*---------------------------------------------------------------------------*/
/*
 * Installs the RTOS interrupt handlers and starts the peripherals.
 */
static void prvHardwareSetup( void );
static void prvServoSetup( void );

/*———————————————————jhgfds,mgfdfhjmng——————————————————*/
int main( void )
{   
static QueueHandle_t xDecoderServoQueue = NULL;    /* queue decoder -> servo */
static QueueHandle_t xKeypadDecoderQueue = NULL;   /* queue keypad -> decoder */
static QueueHandle_t xWPMServoQueue = NULL;        /* queue WPM -> servos */
    
xDecoderParams_t    xDParams;       /* params to the decoder task */
xKeypadParams_t     xKParams;       /* params to the keypad task */
xServoInputQueues_t xServoInputs;   /* the queues to imput into the servo task */
xWPMParams_t        xWPMParams;     /* params to the WPM task */
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	prvHardwareSetup();
    
    /* The tasks return their input queues, so they must be started back to front in the pipeline */
    
    /* start the servo task and get it's input queues */
    xStartServoTasks( mainSERVO_TASK_PRIORITY, &xServoInputs );
    xWPMServoQueue = xServoInputs.pxFromWPM;
    xDecoderServoQueue = xServoInputs.pxFromKeypad;
    
    /* the decoder and the WPM will use an input queue each */
    xDParams.pxDecoderOutputQueue = &xDecoderServoQueue;
    xWPMParams.pxServoInputQueue = &xWPMServoQueue;
    
    /* start the decoder task */
    xKeypadDecoderQueue = xStartDecoderTask( mainDECODER_TASK_PRIORITY, *xDParams );
    xKParams.pxOutputQueue = &xKeypadDecoderQueue;
    
    //xStartWPMTask( mainWPM_TASK_PRIORITY, *xWPMParams );
    
    xStartKeypadTask( mainKEYPAD_TASK_PRIORITY, *xKParams );
    
    /* 9600 baudrate. This will go up to full speed!  */
    // this functionality isn't fully defined yet
    // vAltStartComTestTasks( mainCOM_TEST_TASK_PRIORITY - 1, 9600 );

	/* Will only get here if there was insufficient memory to create the idle
    task.  The idle task is created within vTaskStartScheduler(). */
	vTaskStartScheduler( );

	/* Should never reach here as the kernel will now be running.  If
	vTaskStartScheduler() does return then it is very likely that there was
	insufficient (FreeRTOS) heap space available to create all the tasks,
	including the idle task that is created within vTaskStartScheduler() itself. */
	for( ;; );
}

/*---------------------------------------------------------------------------*/
static void prvServoSetup( void )
{
const uint16_t usMidPoint = usGetMidPoint();

    baseRotationPWM_Start();
    baseElevationPWM_Start();
    elbowPWM_Start();
    wristPitchPWM_Start();
    wristRollPWM_Start();
    grabberPWM_Start();
   
    /* init the servo to middle */
    baseRotationPWM_WriteCompare( usMidPoint );
    baseElevationPWM_WriteCompare( usMidPoint );
    elbowPWM_WriteCompare( usMidPoint );
    wristPitchPWM_WriteCompare( usMidPoint );
    wristRollPWM_WriteCompare( usMidPoint );
    grabberPWM_WriteCompare( usMidPoint );

    /* the arms is in the mid point position - so put it in the storage area */
    arm_position_t xTempPosition = {
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint,
        usMidPoint
    };
    
    vSetCurrentArmPosition( xTempPosition );
    //vSetCurrentPosition( usMidPoint );
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

	/* Start the UART. */
	UART_Start();
    
    /* Start the pwm, as it comprises of a clock and the PWM module both need doing */
    pwmClock_Start();

    /* start the servos */
    prvServoSetup();

    builtInLED_Write(1);
    
    // not bothered about lcd for now
    //LCD_Start();
    //LCD_DisplayOn();

}

/*---------------------------------------------------------------------------*/
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
/* stop warnings */
( void )pxTask;

	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
    UART_PutString("Stack Overflow from ");
    UART_PutString(pcTaskName);
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
