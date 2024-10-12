/*! @mainpage Guia 2 Ejercicio 3
 *
 * @section Descripción
 *
 * Medidor de distancia por ultrasonido
 * 
 * Se añade al ejercicio anterior el envio de datos por UART
 * para observarlos en una terminal en la PC. Además, se replican
 * las funcionalidades de las teclas 1 y 2 con las teclas 'O' y 'H'
 *
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

#define CONFIG_BLINK_PERIOD_TIMER_A 1000000 /**< Periodo del timer A */
#define CONFIG_BLINK_PERIOD_TIMER_B 500000 /**< Periodo del timer B */

/*==================[internal data definition]===============================*/
/**
 * @brief Task handle de la tarea que muestra la medida en la pantalla
 */
TaskHandle_t taskMostrarMedida_task_handle = NULL;


/**
 * @brief Task handle de la tarea que toma las medidas de distancia
 */
TaskHandle_t taskTomarMedida_task_handle = NULL;

/**
 * @brief Task handle de la tarea UART
 */
TaskHandle_t uart_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/** 
 * @fn void FuncTimerMostrarMedida(void* param)
 * @brief Función invocada en la interrupción del Timer A para mostrar medida.
 * Notifica a la tarea de mostrar la medida y la tarea UART.
 * @param param Parámetro no utilizado.
 */
void FuncTimerMostrarMedida(void* param){
    vTaskNotifyGiveFromISR(taskMostrarMedida_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(uart_task_handle,pdFALSE);  	
}

/**
 * @fn void FuncTimerTomarMedida(void* param)
 * @brief Función invocada por la interrupción del timer B para notificar a la tarea de toma de medida.
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerTomarMedida(void* param){
    vTaskNotifyGiveFromISR(taskTomarMedida_task_handle, pdFALSE);    	
}

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

/**
 * @fn void UartTask(void *pvParameter)
 * @brief Tarea que envía la medida de distancia por UART.
 * Envía la medida de distancia en centímetros al puerto UART cuando el sistema está encendido.
 * @param pvParameter Parámetro no utilizado.
 */
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

/**
 * @fn taskTomarMedida(void *pvParameter)
 * @brief Tarea que toma las medidas de distancia utilizando el sensor ultrasónico
 *
 * La medida se almacena en la variable global MEDIDA.
 *
 * @param pvParameter Parámetro de la tarea (no utilizado)
 */
static void taskTomarMedida(void *pvParameter)
{
	while(true)
	{
		MEDIDA = HcSr04ReadDistanceInCentimeters();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @fn void detectarTeclas()
 * @brief Función para detectar las teclas ingresadas por UART.
 * Activa o desactiva el sistema (ON/OFF) y el modo HOLD al recibir comandos por UART.
 */
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

/**
 * @fn void app_main(void)
 * @brief Función principal del programa.
 * 
 * Inicializa los periféricos, timers, UART y crea las tareas para tomar y mostrar medidas.
 */
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

	/*Creación de tareas*/
	xTaskCreate(&taskTomarMedida, "Tomar Medida", 2048, NULL, 5, &taskTomarMedida_task_handle); 
	xTaskCreate(&taskMostrarMedida, "Manejar Leds", 2048, NULL, 5, &taskMostrarMedida_task_handle);
	xTaskCreate(&UartTask, "UART", 512, &myUart, 5, &uart_task_handle);
	
	/*Comienzo de timers*/
  	TimerStart(timer_mostrar.timer);
	TimerStart(timer_tomar_medida.timer);
}
/*==================[end of file]============================================*/