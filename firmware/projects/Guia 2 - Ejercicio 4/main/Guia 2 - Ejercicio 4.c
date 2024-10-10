/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TIMER_A 20000
#define BUFFER_SIZE 231
uint16_t VOLTAJE = 0;
TaskHandle_t conversorAD_task_handle_ = NULL;
/*==================[internal data definition]===============================*/
void FuncTimerConversor(void* param)
{
    vTaskNotifyGiveFromISR(conversorAD_task_handle_, pdFALSE);    	
}

/*==================[internal functions declaration]=========================*/
static void conversionAD_task(void* param){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH2, &VOLTAJE);
		UartSendString(UART_PC, (char*)UartItoa(VOLTAJE,10));
		UartSendString(UART_PC, "\r\n");
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
		 /* Inicialización de timers */
    timer_config_t timer_conversor = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TIMER_A,
        .func_p = FuncTimerConversor, //Aca va la funcion de interrupcion
        .param_p = NULL
    };
    TimerInit(&timer_conversor);
	TimerStart(timer_conversor.timer);

	analog_input_config_t conversorAD = {
		.input = CH2,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&conversorAD);

	serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 57600,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);
	xTaskCreate(&conversionAD_task, "ConversorAD", 2048, NULL, 5, &conversorAD_task_handle_);
}
/*==================[end of file]============================================*/