/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Local includes */
#include "ripe.h"

/* Macros */
#define UNUSED_ARG(var) (var = var)

/* Halt device */
#define HALT_DEV() for (;;);

/*
 * printf() output uses the UART.
 * These constants define the addresses of the required UART registers.
 */
#define UART0_ADDRESS (0x40004400UL)
#define UART0_DATA (*(((volatile uint32_t *)(UART0_ADDRESS + 4UL))))
#define UART0_STATE (*(((volatile uint32_t *)(UART0_ADDRESS + 0UL))))
#define UART0_CTRL (*(((volatile uint32_t *)(UART0_ADDRESS + 8UL))))
#define UART0_BAUDDIV (*(((volatile uint32_t *)(UART0_ADDRESS + 16UL))))
#define TX_BUFFER_MASK (1UL)

/*
 * Printf() output is sent to the serial port.  Initialise the serial hardware.
 */
static void prvUARTInit(void);

/*-----------------------------------------------------------*/

/* Priorities at which tasks are created. */
#define mainUSER_ECHO_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/*-----------------------------------------------------------*/

/*
 * Tasks as described in the comments at the top of this file.
 */
static void vUserEchoReceivedCommandTask(void *parameters);

/*-----------------------------------------------------------*/

void main(void)
{
	/* Hardware initialisation.  printf() output uses the UART for IO. */
	prvUARTInit();

	/* Print something just to check that everything works as expected */
	printf("UART Initialized\r\n");

	/* Create echo receiver task */
	xTaskCreate(vUserEchoReceivedCommandTask,		 /* The function that implements the task. */
				"Echo Received Command Task",		 /* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE,			 /* The size of the stack to allocate to the task. */
				NULL,								 /* The parameter passed to the task - not used in this case. */
				mainUSER_ECHO_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
				NULL);								 /* The task handle is not required, so NULL is passed. */

	/* Start the tasks. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

#define SERIAL_BUFFER_LEN 50
uint8_t serial_buffer[SERIAL_BUFFER_LEN] = {0};

static void vUserEchoReceivedCommandTask(void *parameters)
{
	char attack_code[4];
	struct param_t attack_params;

	UNUSED_ARG(parameters);

	printf("DEVICE->ONLINE\n");
	scanf("%s", serial_buffer);

	if (strncmp("HOST->", (const char *)serial_buffer, 6) != 0)
	{
		printf("DEVICE->ERROR\n");
		HALT_DEV();
	}

	strncpy(attack_code, (const char *)(serial_buffer + 6), 4);
	number_to_attack_params(atoi(attack_code), &attack_params);

	// if the attack is not possible, send a nope

	if (!attack_possible(&attack_params))
		printf("DEVICE->NOPE");
	else
		attack(&attack_params);

	printf("DEVICE->DONE");
}

void main_SVC_Handler(void)
{
	//
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */

	printf("\r\n\r\nStack overflow in %s\r\n", pcTaskName);
	portDISABLE_INTERRUPTS();

	for (;;)
		;
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

void vAssertCalled(const char *pcFileName, uint32_t ulLine)
{
	volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	/* Called if an assertion passed to configASSERT() fails.  See
	http://www.freertos.org/a00110.html#configASSERT for more information. */

	printf("ASSERT! Line %d, file %s\r\n", (int)ulLine, pcFileName);

	taskENTER_CRITICAL();
	{
		/* You can step out of this function to debug the assertion by using
		the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		value. */
		while (ulSetToNonZeroInDebuggerToContinue == 0)
		{
			__asm volatile("NOP");
			__asm volatile("NOP");
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

static void prvUARTInit(void)
{
	UART0_BAUDDIV = 16;
	UART0_CTRL = 1;
}

/*-----------------------------------------------------------*/

int __write(int iFile, char *pcString, int iStringLength)
{
	int iNextChar;

	/* Avoid compiler warnings about unused parameters. */
	(void)iFile;

	/* Output the formatted string to the UART. */
	for (iNextChar = 0; iNextChar < iStringLength; iNextChar++)
	{
		while ((UART0_STATE & TX_BUFFER_MASK) != 0)
			;
		UART0_DATA = *pcString;
		pcString++;
	}

	return iStringLength;
}
