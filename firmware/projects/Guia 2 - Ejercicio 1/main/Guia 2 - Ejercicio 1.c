/*! @mainpage Guia 2 Ejercicio 1
 *
 * @section Descripción
 *
 * Medidor de distancia por ultrasonido
 * 
 * Esta aplicación mide la distancia utilizando un sensor ultrasónico, controla el encendido de LEDs
 * en función de la distancia medida y permite el manejo de teclas para activar o desactivar la medición
 * y retener la última medida mostrada en pantalla.
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
/**
 * @brief Almacena el valor de la distancia medida
 */
int MEDIDA;

/**
 * @brief Estado de encendido del sistema de medición
 */
bool ON = true;

/**
 * @brief Estado de retención de la última medida mostrada en pantalla
 */
bool HOLD = false;

/**
 * @brief Periodo de parpadeo de los LEDs en milisegundos
 */
#define CONFIG_BLINK_PERIOD_LED 500

/**
 * @brief Retardo para mostrar la medida en la pantalla en milisegundos
 */
#define RETARDO_MOSTRAR 500

/**
 * @brief Retardo para tomar una nueva medida en milisegundos
 */
#define RETARDO_MEDIR 1000

/**
 * @brief Retardo para leer el estado de las teclas en milisegundos
 */
#define RETARDO_TECLAS 300

/*==================[internal data definition]===============================*/

/**
 * @brief task handle de la tarea que muestra la medida en la pantalla
 */
TaskHandle_t taskMostrarMedida_task_handle = NULL;

/**
 * @brief task handle de la tarea que maneja las teclas
 */
TaskHandle_t taskManejarTeclas_task_handle = NULL;

/**
 * @brief task handle de la tarea que toma las medidas de distancia
 */
TaskHandle_t taskTomarMedida_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn static void manejarLeds()
 * @brief Controla el encendido de los LEDs según la distancia medida
 *
 * Si la distancia es menor a 10 cm, todos los LEDs están apagados.
 * Si la distancia está entre 10 y 20 cm, se enciende el LED_1.
 * Si la distancia está entre 20 y 30 cm, se encienden los LEDs 1 y 2.
 * Si la distancia es mayor a 30 cm, se encienden todos los LEDs.
 */
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
/**
 * @fn static void taskMostrarMedida(void *pvParameter)
 * @brief Tarea que muestra la medida en la pantalla y maneja los LEDs
 *
 * Si el sistema está encendido (ON), se muestra la medida en la pantalla y se controlan los LEDs
 * según la distancia medida. Si el sistema está en modo HOLD, la medida no se actualiza.
 *
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */
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
/**
 * @fn taskTomarMedida(void *pvParameter)
 * @brief Tarea que toma las medidas de distancia utilizando el sensor ultrasónico
 * La medida se almacena en la variable global MEDIDA.
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */
static void taskTomarMedida(void *pvParameter)
{
	while(true)
	{
		printf("tarea tomar medida\n");
		MEDIDA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(RETARDO_MEDIR / portTICK_PERIOD_MS);
	}
}
/**
 * @fn static void taskManejarTeclas(void *pvParameter)
 * @brief Tarea que maneja las teclas para encender/apagar el sistema o activar/desactivar HOLD
 * Detecta el estado de las teclas y cambia las variables ON y HOLD en función de las entradas.
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */
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

/**
 * @fn void app_main(void)
 * @brief Función principal de la aplicación
 *
 * Inicializa los LEDs, la pantalla LCD, el sensor ultrasónico y las teclas, y crea las tareas necesarias
 * para manejar el sistema de medición, los LEDs y las teclas.
 */
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