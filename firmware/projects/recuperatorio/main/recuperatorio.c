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
/*! @brief Variable que almacena el valor de la distancia medida actual*/
int DISTANCIA;

/*! @brief Variable que almacena el valor de la distancia medida anterior*/
int DISTANCIA_ANTERIOR;

/*! @brief Variable que almacena el valor de la velocidad del vehículo*/
float VELOCIDAD; 

/*! @brief Período del temporizador para la lectura de la velocidad. En este caso 0,1 s (100000 us)*/
#define CONFIG_BLINK_PERIOD_TIMER_B 100000 
/*==================[internal data definition]===============================*/

/*! @brief Task handle para la tarea Medir Velocidad */
TaskHandle_t taskMedirVelocidad_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void FuncTimerMedirVelocidad(void* param)
 * @brief Función invocada por la interrupción del timer B para notificar a la tarea de toma de velocidad.
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerMedirVelocidad(void* param)
{
    vTaskNotifyGiveFromISR(taskMedirVelocidad_task_handle, pdFALSE);    	
}

/**
 * @fn void controlarLeds()
 * @brief Controla el encendido de los LEDs según la velocidad medida
 *
 * Si la velocidad es mayor a 8 m/s, se enciende el LED3 ()
 * Si la velocidad se encuentra entre 0 m/s y 8 m/s, se enciende el LED2
 * Si el vehículo está detenido, se enciende el LED1
 */
void controlarLeds()
{
		if (VELOCIDAD > 8)
		{
			LedOn(LED_3);
			LedOff(LED_2);
			LedOff(LED_1);
		}
		else if(VELOCIDAD > 0 && VELOCIDAD <= 8)
		{
			LedOff(LED_3);
			LedOn(LED_2);
			LedOff(LED_1);
		}
		else if(VELOCIDAD == 0) //Asumo que la velocidad no puede ser negativa por que el vehículo no retrocede
		{
			LedOff(LED_3);
			LedOff(LED_2);
			LedOn(LED_1);
		}
}
/**
 * @fn taskMedirVelocidad(void *pvParameter)
 * @brief Tarea que toma las medidas de distancia utilizando el sensor ultrasónico y además utiliza estas mismas para calcular la velocidad del vehículo
 *
 * La medida se almacena en la variable global .
 *
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */

static void taskMedirVelocidad(void *pvParameter)
{
	while(true)
	{
		DISTANCIA_ANTERIOR = DISTANCIA;
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		LedsOffAll();

		if (DISTANCIA < 1000) //Si el vehículo se encuentra a menos de 1000 cm (10 m)...
		{
			VELOCIDAD = (DISTANCIA_ANTERIOR - DISTANCIA)/(0,1*100); 		//delta de desplazamiento sobre el tiempo que 																	
			controlarLeds();																		//transcurrió entre ambos puntos (es decir el tiempo de muestreo)
		}																	//Además se divide por 100 para convertir de cm a m
			
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Inicialización de periféricos*/
	HcSr04Init(GPIO_3, GPIO_2); 
	LedsInit();

	/*Timers */
	timer_config_t timer_tomar_distancia = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerMedirVelocidad,
        .param_p = NULL
    };

	TimerInit(&timer_tomar_distancia);
	TimerStart(timer_tomar_distancia.timer);

	xTaskCreate(&taskMedirVelocidad, "Tomar Medida", 2048, NULL, 5, &taskMedirVelocidad_task_handle); 

}
/*==================[end of file]============================================*/