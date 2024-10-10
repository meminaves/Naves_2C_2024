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
#define CONFIG_BLINK_PERIOD_TIMER_B 40000
#define BUFFER_SIZE 231


uint16_t VOLTAJE = 0;
TaskHandle_t conversorAD_task_handle_ = NULL;
TaskHandle_t conversorDA_task_handle_ = NULL;

/*==================[internal data definition]===============================*/
void FuncTimerConversor(void* param)
{
    vTaskNotifyGiveFromISR(conversorAD_task_handle_, pdFALSE);      	
}
void FuncTimerECG(void *param)
{
	vTaskNotifyGiveFromISR(conversorDA_task_handle_, pdFALSE); 
}
TaskHandle_t main_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
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
static void conversionDA_task(void*param)
{
	uint16_t  contador_ecg= 0;
	while (true)
	{
		printf("Entra");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		

		if (contador_ecg<BUFFER_SIZE)
		{
			AnalogOutputWrite(ecg[contador_ecg]);
			contador_ecg++;
		}
		else
		{
			contador_ecg = 0; 
		}
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
	    timer_config_t timer_ECG = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerECG, //Aca va la funcion de interrupcion
        .param_p = NULL
    };
    TimerInit(&timer_conversor);
	TimerStart(timer_conversor.timer);
	
	TimerInit(&timer_ECG);
	TimerStart(timer_ECG.timer);

	analog_input_config_t conversorAD = {
		.input = CH2,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&conversorAD);
	AnalogOutputInit();

	serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);

	xTaskCreate(&conversionAD_task, "ConversorAD", 2048, NULL, 5, &conversorAD_task_handle_);
	xTaskCreate(&conversionDA_task, "ConversorDA", 2048, NULL, 5, &conversorDA_task_handle_);
}
/*==================[end of file]============================================*/