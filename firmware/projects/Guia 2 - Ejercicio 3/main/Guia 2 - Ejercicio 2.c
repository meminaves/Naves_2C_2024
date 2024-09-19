/*! @mainpage Guia 2 Ejercicio 3
 *
 * @section Descripción
 *
 * Medidor de distancia por ultrasonido
 *
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
 * | 05/09/2023 | Document creation		                         |
 *
 * @author Maria Emilia Naves 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/
int MEDIDA;
bool ON = true;
bool HOLD = false;

#define CONFIG_BLINK_PERIOD_LED 500
#define RETARDO_MOSTRAR 500
#define RETARDO_MEDIR 1000
#define RETARDO_TECLAS 300
#define CONFIG_BLINK_PERIOD_LED_1_US 1000000

/*==================[internal data definition]===============================*/
TaskHandle_t taskMostrarMedida_task_handle = NULL;
TaskHandle_t taskTomarMedida_task_handle = NULL;
TaskHandle_t uart_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerTeclas(void* param){
    vTaskNotifyGiveFromISR(taskMostrarMedida_task_handle, pdFALSE);  	
}

void FuncTimerTomarMedida(void* param){
    vTaskNotifyGiveFromISR(taskTomarMedida_task_handle, pdFALSE);    	
}

void tecla1(){
	ON = !ON;
}

void tecla2(){
	HOLD = !HOLD;
}

static void manejarLeds()
{
		LedsOffAll();

		if (MEDIDA < 10)
		{
			LedsOffAll();
		}
		else if(MEDIDA>=10 && MEDIDA <= 20)
		{
			printf("LED_1 ON\n");
			LedOn(LED_1);
		}
		else if(MEDIDA>=20 && MEDIDA <= 30)
		{
			printf("LED_1 ON\n");
			LedOn(LED_1);

			printf("LED_2 ON\n");
			LedOn(LED_2);
		}
		else 
		{
			printf("LED_ON_ALL\n");
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
}

void UartTask(void *pvParameter)
{
	while(true)
	{
		UartSendString(UART_PC, "Hello World");
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
static void taskMostrarMedida(void *pvParameter)
{
	while (true)
	{
		printf("tarea mostrar medida\n");
		if (ON == true)
		{
			manejarLeds();
			printf("hold;%d\n", HOLD); 
			
			if (HOLD == false)
			{
				LcdItsE0803Write(MEDIDA);
				printf("medida;%d\n", MEDIDA); 
			}
		}
		else 
		{
				LcdItsE0803Off();
				LedsOffAll();
		}
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

static void taskTomarMedida(void *pvParameter)
{
	while(true)
	{
		printf("tarea tomar medida\n");
		MEDIDA = HcSr04ReadDistanceInCentimeters();
		UartItoa(MEDIDA,10);
		UartSendString(UART_PC,MEDIDA);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	LedsInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	LcdItsE0803Write(77);

	 /* Inicialización de timers */
    timer_config_t timer_teclas = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_LED_1_US,
        .func_p = FuncTimerTeclas,
        .param_p = NULL
    };
    TimerInit(&timer_teclas);

    timer_config_t timer_tomar_medida = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_LED_1_US,
        .func_p = FuncTimerTomarMedida,
        .param_p = NULL
    };
	TimerInit(&timer_tomar_medida);
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&my_uart);

	xTaskCreate(&taskTomarMedida, "Tomar Medida", 2048, NULL, 5, &taskTomarMedida_task_handle); 
	xTaskCreate(&taskMostrarMedida, "Manejar Leds", 2048, NULL, 5, &taskMostrarMedida_task_handle);
	xTaskCreate(&UartTask, "UART", 512, &my_uart, 5, &uart_task_handle);

	SwitchActivInt(SWITCH_1,*tecla1,NULL); 
	SwitchActivInt(SWITCH_2,*tecla2,NULL);

  	TimerStart(timer_teclas.timer);
	TimerStart(timer_tomar_medida.timer);
}
/*==================[end of file]============================================*/