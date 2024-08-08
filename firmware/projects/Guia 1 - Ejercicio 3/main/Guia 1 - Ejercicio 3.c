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

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
//Realice un función que reciba un puntero a una estructura LED como la que se muestra a continuación: 

#define CONFIG_BLINK_PERIOD 200
#define ON 1
#define OFF 2
#define TOGGLE 3
#define DELAY 100

struct leds
{
	uint8_t mode;         //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;     //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;     //indica el tiempo de cada ciclo

} my_leds; 

void manejarLeds(struct leds *ptrLeds);

void app_main(void){
	printf("Hello world!\n");

	struct leds ptrLEDs;
	LedsInit();
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

		ptrLEDs.mode = TOGGLE;
		ptrLEDs.n_ciclos = 2000;
		ptrLEDs.n_led = 2;
		ptrLEDs.periodo = 500;   
	
	manejarLeds(&ptrLEDs);
}
/*==================[end of file]============================================*/

void manejarLeds(struct leds *ptrLeds)
{
	led_t led_n;
	LedsInit();

	if(ptrLeds->n_led == 1)
	{
		led_n = LED_1;
	}
	else if (ptrLeds->n_led == 2)
	{
		led_n = LED_2;
	}
	else if (ptrLeds->n_led == 3)
	{
		led_n = LED_3;
	}
	else
	{
		led_n = LED_1;
	}

	switch(ptrLeds->mode)
	{
		case ON: //ON
		{
			LedOn(led_n); 
			printf("LED encendido");
		}
			break;

		case OFF: //OFF
		{
			LedOff(led_n);
		}
			break;

		case TOGGLE:
		{
			for(int i = 0; i < ptrLeds->n_ciclos; i++)
			{
				LedToggle(led_n);

				int retardo_aux = ptrLeds->periodo/DELAY;
				
				printf("ciclo");

				for (int j = 0; j < retardo_aux; j++)
				{
					vTaskDelay(DELAY / portTICK_PERIOD_MS);
				}

			}		
		}
			break;

		default:
			printf("Opcion mode no reconocida");
	};

}