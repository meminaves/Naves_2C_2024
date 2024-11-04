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
#include "led.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
int DISTANCIA;

/*! @brief Variable que almacena el valor del voltaje leído */
float VOLTAJE_X = 0;

/*! @brief Variable que almacena el valor del voltaje leído */
float VOLTAJE_Y = 0;

/*! @brief Variable que almacena el valor del voltaje leído */
float VOLTAJE_Z = 0;

float AC_X;

float AC_Y;

float AC_Z;

float AC_total;

bool ON = true;

#define SENSIBILIDAD_AC 0.3f

#define GPIO_BUZZER GPIO_6

#define DELAY_PRECAUCION 1000 //1 s

#define DELAY_PELIGRO 500 //0,5 s

#define AC_CAIDA 4.0f

#define CONFIG_BLINK_PERIOD_TIMER_B 500000 /**< Periodo del timer B */
#define CONFIG_BLINK_PERIOD_TIMER_A 10000 //10000 us => frecuencia de 100 Hz
#define CONFIG_BLINK_PERIOD_TIMER_C 8000 

typedef enum estados_de_distancia
{
	DISTANCIA_ACEPTABLE,
	DISTANCIA_PRECAUCION,
	DISTANCIA_PELIGRO,

} estado_distancia;

estado_distancia ESTADO_DISTANCIA_ACTUAL;

/*==================[internal data definition]===============================*/

TaskHandle_t taskTomarDistancia_task_handle = NULL;

TaskHandle_t  taskMedirAceleracion_task_handle = NULL;

TaskHandle_t taskManejarSistema_task_handle = NULL;

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

void FuncTimerMedirAceleracion(void* param)
{
    vTaskNotifyGiveFromISR(taskMedirAceleracion_task_handle, pdFALSE);    	
}

void FuncTimerManejarSistema(void* param)
{
    vTaskNotifyGiveFromISR(taskManejarSistema_task_handle, pdFALSE);    	
}

void inicializarBuzzer()
{
	void GPIOInit(GPIO_6, GPIO_OUTPUT);
}

void detectarEstadoDeDistancia()
{
    if (DISTANCIA >= 5)
    {
        ESTADO_DISTANCIA_ACTUAL = DISTANCIA_ACEPTABLE;
    }
    else if (DISTANCIA < 5 && DISTANCIA > 3)
    {
        ESTADO_DISTANCIA_ACTUAL = DISTANCIA_PRECAUCION;
    }
    else if (DISTANCIA <= 3)
	{
		ESTADO_DISTANCIA_ACTUAL = DISTANCIA_PELIGRO;
	}
}
void inicializarAcelerometro()
{
	analog_input_config_t presion_config1 = {
        .input = CH0,
        .mode = ADC_SINGLE,
    };
		analog_input_config_t presion_config2 = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
		analog_input_config_t presion_config3 = {
        .input = CH2,
        .mode = ADC_SINGLE,
    };

	AnalogInputInit(&presion_config1);
	AnalogInputInit(&presion_config2);
	AnalogInputInit(&presion_config3);
}



void encenderBuzzerPrecaucion()
{
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(DELAY_PRECAUCION / portTICK_PERIOD_MS);
}

void encenderBuzzerPeligro()
{
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(DELAY_PELIGRO / portTICK_PERIOD_MS);
}

void apagarBuzzer()
{
	GPIOOff(GPIO_BUZZER);
}

void manejarPerifericos()
{
		LedsOffAll();

		if (ESTADO_DISTANCIA_ACTUAL == DISTANCIA_ACEPTABLE)
		{
			apagarBuzzer();
			
			LedOn(LED_1); //LED verde
		}
		else if(ESTADO_DISTANCIA_ACTUAL == DISTANCIA_PRECAUCION)
		{
			LedOn(LED_1);
			LedOn(LED_2); //LED Amarillo

			encenderBuzzerPrecaucion();
		}
		else if(ESTADO_DISTANCIA_ACTUAL == DISTANCIA_PELIGRO)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3); //LED rojo

			encenderBuzzerPeligro();
		}
}

void enviarDatosApp()
{
	if (ESTADO_DISTANCIA_ACTUAL == DISTANCIA_PRECAUCION)
	{
		UartSendString(UART_CONNECTOR, "Precaucion, vehiculo cerca\n");
	}
	else if (ESTADO_DISTANCIA_ACTUAL == DISTANCIA_PELIGRO)
	{
		UartSendString(UART_CONNECTOR, "Peligro, vehiculo cerca\n");
	}

	if (AC_total > AC_CAIDA)
	{
		UartSendString(UART_CONNECTOR, "Caida detectada\n");
	}
}

static void taskTomarDistancia(void *pvParameter)
{
	while(true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}
static void taskManejarSistema(){

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(ON == true)
        {
			detectarEstadoDeDistancia();
            manejarPerifericos(); 
            enviarDatosApp();
        }
        else
        {
            LedsOffAll();
			apagarBuzzer();
        }

	}
}

static void taskMedirAceleracion()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		AnalogInputReadSingle(CH0, &VOLTAJE_X); //lecturas en mV
		AnalogInputReadSingle(CH1, &VOLTAJE_Y);
		AnalogInputReadSingle(CH2, &VOLTAJE_Z);

		VOLTAJE_X = VOLTAJE_X/1000; //PASO A VOLTS
		VOLTAJE_Y = VOLTAJE_Y/1000;
		VOLTAJE_Z = VOLTAJE_Z/1000;

		AC_X = (VOLTAJE_X - 1.65)*(1/SENSIBILIDAD_AC);
		AC_Y = (VOLTAJE_Y - 1.65)*(1/SENSIBILIDAD_AC);
		AC_Z = (VOLTAJE_Z - 1.65)*(1/SENSIBILIDAD_AC);

		AC_total = AC_X + AC_Y + AC_Z;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	
	inicializarBuzzer();
	inicializarAcelerometro();

	    timer_config_t timer_tomar_distancia = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerTomarDistancia,
        .param_p = NULL
    };

		timer_config_t timer_medir_aceleracion = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_A,
        .func_p = FuncTimerMedirAceleracion,
        .param_p = NULL
    };

		timer_config_t timer_sistema= {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_C,
        .func_p = FuncTimerManejarSistema,
        .param_p = NULL
    };

	TimerInit(&timer_medir_aceleracion);
	TimerStart(timer_medir_aceleracion.timer);

	TimerInit(&timer_tomar_distancia);
	TimerStart(timer_tomar_distancia.timer);

	TimerInit(&timer_sistema);
	TimerStart(timer_sistema.timer);

	serial_config_t myUart = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);

	xTaskCreate(&taskTomarDistancia, "Tomar Medida", 2048, NULL, 5, &taskTomarDistancia_task_handle); 
	xTaskCreate(&taskMedirAceleracion, "Medir Aceleracion", 2048, NULL, 5, &taskMedirAceleracion_task_handle); 
	xTaskCreate(&taskManejarSistema, "Manejar Sistema", 2048, NULL, 5, &taskManejarSistema_task_handle); 
}
/*==================[end of file]============================================*/