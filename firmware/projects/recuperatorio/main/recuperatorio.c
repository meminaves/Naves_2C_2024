/*! @mainpage Recuperatorio EP
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 * 
 * | Peripheral    | ESP32        |
 * |---------------|--------------|
 * |  HC-SR04 Trig | GPIO_3       |
 * |  HC-SR04 Echo | GPIO_2       |
 * |  LED 1        | GPIO_20      |
 * |  LED 2        | GPIO_21      |
 * |  LED 3        | GPIO_22      |
 * |  Switch 1     | GPIO_4       |
 * |  Switch 2     | GPIO_15      |
 * |  UART         | Conexión UART_PC    |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
 *
 * @author Maria Emilia Naves
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "led.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/*! @brief Variable que almacena el valor de la distancia medida */
int DISTANCIA;

/*! @brief Período del temporizador para la lectura de la distancia. En este caso 0,1 s*/
#define CONFIG_BLINK_PERIOD_TIMER_B 100000 
/*==================[internal data definition]===============================*/

/*! @brief Task handle para la tarea Tomar Distancia */
TaskHandle_t taskTomarDistancia_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void FuncTimerTomarDistancia(void* param)
 * @brief Función invocada por la interrupción del timer B para notificar a la tarea de toma de distancia.
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerTomarDistancia(void* param)
{
    vTaskNotifyGiveFromISR(taskTomarDistancia_task_handle, pdFALSE);    	
}
/**
 * @fn taskTomarDistancia(void *pvParameter)
 * @brief Tarea que toma las medidas de distancia utilizando el sensor ultrasónico
 *
 * La medida se almacena en la variable global Distancia.
 *
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */
static void taskTomarDistancia(void *pvParameter)
{
	while(true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Inicialización de periféricos*/
	HcSr04Init(GPIO_3, GPIO_2); 


	/*Timers */
	timer_config_t timer_tomar_distancia = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerTomarDistancia,
        .param_p = NULL
    };

	TimerInit(&timer_tomar_distancia);
	TimerStart(timer_tomar_distancia.timer);

	xTaskCreate(&taskTomarDistancia, "Tomar Medida", 2048, NULL, 5, &taskTomarDistancia_task_handle); 

}
/*==================[end of file]============================================*/