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

/*! @brief Variable que almacena el valor de la velocidad maxima del vehículo*/
float VELOCIDAD_MAX;

/*! @brief Variable que almacena el peso del vehículo*/
int PESO;

/*! @brief Período del temporizador para la lectura de la velocidad. En este caso 0,1 s (100000 us)*/
#define CONFIG_BLINK_PERIOD_TIMER_B 100000 

/*! @brief Período del temporizador para la lectura del peso del vehiculo. En este caso 5000 us (200 muestras por segundo)*/
#define CONFIG_BLINK_PERIOD_TIMER_A 5000

/*! @brief Tamaño de los buffer que almacenan las mediciones de peso de ambas galgas*/
#define BUFFER_SIZE 50

/*! @brief Canal destinado a la lectura ADC de la galga 1*/
#define CH_GALGA1 CH0

/*! @brief Canal destinado a la lectura ADC de la galga 2*/
#define CH_GALGA2 CH1

/*! @brief Vector que almacena las 50 mediciones de peso de la galga 1*/
float PESOS_GALGA_1 [BUFFER_SIZE];

/*! @brief Vector que almacena las 50 mediciones de peso de la galga 2*/
float PESOS_GALGA_2 [BUFFER_SIZE];

/*==================[internal data definition]===============================*/

/*! @brief Task handle para la tarea Medir Velocidad */
TaskHandle_t taskMedirVelocidad_task_handle = NULL;

/*! @brief Task handle para la tarea Pesar Vehiculo */
TaskHandle_t taskPesarVehiculo_task_handle = NULL;
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
 * @fn void FuncTimerPesarVehiculo(void* param)
 * @brief Función invocada por la interrupción del timer A para notificar a la tarea Pesar Vehiculo.
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerPesarVehiculo(void* param)
{
    vTaskNotifyGiveFromISR(taskPesarVehiculo_task_handle, pdFALSE);    	
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
		
		if (DISTANCIA < 1000) //Si el vehículo se encuentra a menos de 1000 cm (10 m)...
		{
			VELOCIDAD = (DISTANCIA_ANTERIOR - DISTANCIA)/(0.1*100); 		//delta de desplazamiento sobre el tiempo que 																	
			controlarLeds();												//transcurrió entre ambos puntos (es decir el tiempo de muestreo)
																			//Además se divide por 100 para convertir de cm a m
			if (VELOCIDAD > VELOCIDAD_MAX)
			{
				VELOCIDAD_MAX = VELOCIDAD;
			}
		}																	
		else 
		{
			LedsOffAll(); //si no hay vehículo se apagan los leds
			VELOCIDAD_MAX = -1; //La velocidad no podria ser negativa porque asumo que el vehiculo no retrocede según la consigna
								//Reinicio la v_max para el proximo camión
		}																	
			
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}
/**
 * @fn static void pesarVehiculoTask(void*param)
 * @brief Tarea encargada de pesar el vehículo si este se encuentra detenido
 *
 * Esta tarea pesa el vehículo si se encuentra detenido. Una vez se detecta la velocidad cero, 
 * se proceden a tomar 50 mediciones de cada galga, se promedian, y finalmente se suman y se obtiene el peso.
 *
 * @param param Parámetro no utilizado.
 */
static void pesarVehiculoTask(void*param)
{
	uint16_t  contador = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		uint16_t voltaje_galga_1;
		uint16_t voltaje_galga_2;

		if (VELOCIDAD == 0) //Si el vehículo está detenido...
		{ 			
			if (contador < BUFFER_SIZE)
			{
				AnalogInputReadSingle(CH_GALGA1, &voltaje_galga_1); //salida en mV
				AnalogInputReadSingle(CH_GALGA2, &voltaje_galga_2);

				PESOS_GALGA_1[contador] = (voltaje_galga_1 * 20000)/(3.3*1000); //Paso a volts acá directamente dividendo por mil
				PESOS_GALGA_2[contador] = (voltaje_galga_2 * 20000)/(3.3*1000);

				contador++;
			}
			else
			{
				float Promedio1 = 0;
				float Promedio2 = 0;
				contador = 0;

				for (int i = 0; i < BUFFER_SIZE; i++)
				{
					Promedio1 = PESOS_GALGA_1[i] + Promedio1;
					Promedio2 = PESOS_GALGA_2[i] + Promedio2; 
				}

				float PESO_PROMEDIO_1 = Promedio1 / BUFFER_SIZE;
				float PESO_PROMEDIO_2 = Promedio2 / BUFFER_SIZE;

				PESO = PESO_PROMEDIO_1 + PESO_PROMEDIO_2;
			}
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Inicialización de periféricos*/
	HcSr04Init(GPIO_3, GPIO_2); 
	LedsInit();
	
	/*Inicialización de conversores analógicos-digitales*/
	analog_input_config_t galga_config1 = {
        .input = CH_GALGA1,
        .mode = ADC_SINGLE,
    };
		analog_input_config_t galga_config2 = {
        .input = CH_GALGA2,
        .mode = ADC_SINGLE,
    };

	AnalogInputInit(&galga_config1);
	AnalogInputInit(&galga_config2);

	/*Timers */
	timer_config_t timer_tomar_distancia = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerMedirVelocidad,
        .param_p = NULL
    };

	timer_config_t timer_pesar_vehiculo = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TIMER_A,
        .func_p = FuncTimerPesarVehiculo,
        .param_p = NULL
    };
	TimerInit(&timer_tomar_distancia);
	TimerStart(timer_tomar_distancia.timer);

	TimerInit(&timer_pesar_vehiculo);
	TimerStart(timer_pesar_vehiculo .timer);

	xTaskCreate(&taskMedirVelocidad, "Medir Velocidad", 2048, NULL, 5, &taskMedirVelocidad_task_handle); 
	xTaskCreate(&pesarVehiculoTask, "Pesar Vehiculo", 2048, NULL, 5, &taskPesarVehiculo_task_handle); 

}
/*==================[end of file]============================================*/