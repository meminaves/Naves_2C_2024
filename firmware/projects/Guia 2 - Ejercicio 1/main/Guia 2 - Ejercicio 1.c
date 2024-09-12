/*! @mainpage Guia 2 Ejercicio 1
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
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/
int MEDIDA;
bool ON = true;
bool HOLD = false;

#define CONFIG_BLINK_PERIOD_LED 500
#define RETARDO_MOSTRAR 500
#define RETARDO_MEDIR 1000
#define RETARDO_TECLAS 300

/*==================[internal data definition]===============================*/
TaskHandle_t taskMostrarMedida_task_handle = NULL;
TaskHandle_t taskManejarTeclas_task_handle = NULL;
TaskHandle_t taskTomarMedida_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
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

		vTaskDelay(CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
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
		
		vTaskDelay(RETARDO_MOSTRAR / portTICK_PERIOD_MS);
	}
}

static void taskTomarMedida(void *pvParameter)
{
	while(true)
	{
		printf("tarea tomar medida\n");
		MEDIDA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(RETARDO_MEDIR / portTICK_PERIOD_MS);
	}
}

static void taskManejarTeclas(void *pvParameter)
{
	while(1)    
	{
		printf("tarea manejar teclas\n");
		uint8_t teclas;
    	teclas  = SwitchesRead();
    	switch(teclas)
		{
    		case SWITCH_1:
			printf("Tecla 1\n");
			ON = !ON;

    		break;

    		case SWITCH_2:
			printf("Tecla 2\n");
			HOLD = !HOLD;

    		break;
		}
		vTaskDelay(RETARDO_TECLAS / portTICK_PERIOD_MS);
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

	xTaskCreate(&taskManejarTeclas, "Manejar Teclas", 2048, NULL, 5, &taskManejarTeclas_task_handle);
	xTaskCreate(&taskTomarMedida, "Tomar Medida", 2048, NULL, 5, &taskTomarMedida_task_handle); 
	xTaskCreate(&taskMostrarMedida, "Manejar Leds", 2048, NULL, 5, &taskMostrarMedida_task_handle);
}

/*==================[end of file]============================================*/