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
#define CONFIG_BLINK_PERIOD_TIMER_A 1000000
#define CONFIG_BLINK_PERIOD_TIMER_B 500000

/*==================[internal data definition]===============================*/
TaskHandle_t taskMostrarMedida_task_handle = NULL;
TaskHandle_t taskTomarMedida_task_handle = NULL;
TaskHandle_t uart_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerMostrarMedida(void* param){
    vTaskNotifyGiveFromISR(taskMostrarMedida_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(uart_task_handle,pdFALSE);  	
}

void FuncTimerTomarMedida(void* param){
    vTaskNotifyGiveFromISR(taskTomarMedida_task_handle, pdFALSE);    	
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
			//printf("LED_1 ON\n");
			LedOn(LED_1);
		}
		else if(MEDIDA>=20 && MEDIDA <= 30)
		{
			//printf("LED_1 ON\n");
			LedOn(LED_1);

			//printf("LED_2 ON\n");
			LedOn(LED_2);
		}
		else 
		{
			//printf("LED_ON_ALL\n");
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
}

void UartTask(void *pvParameter)
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if(ON == true)
		{
			UartSendString(UART_PC,(char*)UartItoa(MEDIDA,10));
			UartSendString(UART_PC, " ");
			UartSendString(UART_PC, "cm");
			UartSendString(UART_PC, "\r\n");
		}
	}
}

static void taskMostrarMedida(void *pvParameter)
{
	while (true)
	{
		//printf("tarea mostrar medida\n");
		if (ON == true)
		{
			manejarLeds();
			//printf("hold;%d\n", HOLD); 
			
			if (HOLD == false)
			{
				LcdItsE0803Write(MEDIDA);
				//printf("medida;%d\n", MEDIDA); 
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
		MEDIDA = HcSr04ReadDistanceInCentimeters();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void detectarTeclas()
{
	uint8_t tecla;
	UartReadByte(UART_PC, &tecla);
	switch (tecla)
	{
		case 'O':
			ON = !ON;
			UartSendByte(UART_PC, (char*)&tecla);
			break;
	
		case 'H':
			HOLD = !HOLD;
			UartSendByte(UART_PC, (char*)&tecla);
			break;
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
    timer_config_t timer_mostrar = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TIMER_A,
        .func_p = FuncTimerMostrarMedida,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

    timer_config_t timer_tomar_medida = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerTomarMedida,
        .param_p = NULL
    };
	TimerInit(&timer_tomar_medida);

    //Puerto Serie
		serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = detectarTeclas,
		.param_p = NULL,
	};

	UartInit(&myUart);

	xTaskCreate(&taskTomarMedida, "Tomar Medida", 2048, NULL, 5, &taskTomarMedida_task_handle); 
	xTaskCreate(&taskMostrarMedida, "Manejar Leds", 2048, NULL, 5, &taskMostrarMedida_task_handle);
	xTaskCreate(&UartTask, "UART", 512, &myUart, 5, &uart_task_handle);
	
  	TimerStart(timer_mostrar.timer);
	TimerStart(timer_tomar_medida.timer);
}
/*==================[end of file]============================================*/