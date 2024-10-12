/*! @mainpage Guia 2 Ejercicio 4
 *
 * @section Descripción
 *
 * Programa para medir el voltaje usando el conversor ADC del ESP32 y enviar los datos por UART.
 *
 * El sistema toma una lectura analógica del canal ADC 2, la convierte a un valor digital y la transmite a través de la interfaz UART. 
 * La lectura se realiza a intervalos de tiempo configurados por un temporizador.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ADC_CH2		| 	GPIO_2		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Maria Emilia Naves
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
/*! @brief Período del temporizador en microsegundos */
#define CONFIG_BLINK_PERIOD_TIMER_A 20000 

/*==================[internal data definition]===============================*/

/*! @brief Variable que almacena el valor del voltaje leído */
uint16_t VOLTAJE = 0;

/*! @brief Task handle para la tarea del conversor ADC */
TaskHandle_t conversorAD_task_handle_ = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn void FuncTimerConversor(void* param)
 * @brief Función invocada en la interrupción del temporizador.
 *
 * Esta función es llamada cuando el temporizador A genera una interrupción. 
 * Despierta la tarea encargada de realizar la conversión ADC.
 *
 * @param param Parámetro no utilizado.
 */
void FuncTimerConversor(void* param)
{
    vTaskNotifyGiveFromISR(conversorAD_task_handle_, pdFALSE);    	
}

/**
 * @fn static void conversionAD_task(void* param)
 * @brief Tarea encargada de leer el valor del conversor ADC y enviarlo por UART.
 *
 * Esta tarea espera a ser notificada por la interrupción del temporizador.
 * Una vez notificada, lee el valor analógico del canal ADC 2 y lo convierte 
 * a un valor digital que luego se envía al puerto UART.
 *
 * @param param Parámetro no utilizado.
 */
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

/**
 * @brief Función principal de la aplicación.
 *
 * Esta función inicializa el temporizador, la UART y el ADC.
 * Luego crea la tarea encargada de realizar la conversión ADC periódicamente
 * y enviar los datos por UART.
 */
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

	
	/*Configuración del ADC*/
	analog_input_config_t conversorAD = {
		.input = CH2,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&conversorAD);

	/* Configuración del puerto UART */
	serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 57600,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);

	/* Creación de la tarea para la conversión ADC */
	xTaskCreate(&conversionAD_task, "ConversorAD", 2048, NULL, 5, &conversorAD_task_handle_);
}
/*==================[end of file]============================================*/