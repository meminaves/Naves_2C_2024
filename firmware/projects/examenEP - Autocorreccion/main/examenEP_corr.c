/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Aplicación que implementa una alerta para ciclistas
 *
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral    | ESP32        |
 * |---------------|--------------|
 * |  HC-SR04 Trig | GPIO_7       |
 * |  HC-SR04 Echo | GPIO_8       |
 * |  LED 1        | GPIO_20      |
 * |  LED 2        | GPIO_21      |
 * |  LED 3        | GPIO_22      |
 * |  Buzzer       | GPIO_6       |
 * |  Switch 2     | GPIO_15      |
 * |  UART         | UART_CONNECTION    |
 * | 	ADC_CH0  	| 	GPIO_0		|
 * | 	ADC_CH1  	| 	GPIO_1		|
 * | 	ADC_CH2  	| 	GPIO_2		|
 * | 	ADC_CH2  	| 	GPIO_2		|
 * | 	TX      	| 	GPIO_18		|
 * | 	RX      	| 	GPIO_19	    |

 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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
/*! @brief Variable que almacena la distancia medida por el sensor de ultrasonido (en cm) */
int DISTANCIA;

/*! @brief Variable que almacena el valor del voltaje leído del eje X del acelerómetro */
float VOLTAJE_X = 0;

/*! @brief Variable que almacena el valor del voltaje leído del eje Y del acelerómetro */
float VOLTAJE_Y = 0;

/*! @brief Variable que almacena el valor del voltaje leído del eje Z del acelerómetro */
float VOLTAJE_Z = 0;

/*! @brief Variable que almacena el valor de aceleración del eje X */
float AC_X;

/*! @brief Variable que almacena el valor de aceleración del eje Y */
float AC_Y;

/*! @brief Variable que almacena el valor de aceleración del eje Z */
float AC_Z;

/*! @brief Variable que almacena la sumatoria escalar de aceleración*/
float AC_total;

/*! @brief Variable que determina que el dispositivo está encendido. Podria en un futuro implementarse un boton de apagado/encendido */
bool ON = true;

/**
 * @def SENSIBLIDIDAD_AC
 * @brief Sensibilidad del acelerómetro
 */
#define SENSIBILIDAD_AC 0.3f

/**
 * @def GPIO_BUZZER
 * @brief GPIO destinado al buzzer, en este caso el GPIO_6
 */
#define GPIO_BUZZER GPIO_6

/**
 * @def DELAY_PRECAUCION
 * @brief Delay del buzzer cuando se encuentra en estado de precaución
 */
#define DELAY_PRECAUCION 500 //1 s -> durante 500 ms está encendido y durante los otros 500 ms está apagado

/**
 * @def DELAY_PELIGRO
 * @brief Delay del buzzer cuando se encuentra en estado de peligro
 */
#define DELAY_PELIGRO 250 //0,5 s -> durante 250 ms está encendido y durante los otros 250 ms está apagado

/**
 * @def AC_CAIDA
 * @brief Aceleración para la cual se considera que existe una caída
 */
#define AC_CAIDA 4.0f

/*! @brief Período del temporizador para la lectura de la distancia */
#define CONFIG_BLINK_PERIOD_TIMER_B 500000 

/*! @brief Periodo del temporizador para la medida de aceleración*/
#define CONFIG_BLINK_PERIOD_TIMER_A 10000 //10000 us => frecuencia de 100 Hz

/*! @brief Período del temporizador para el manejo del sistema */
#define CONFIG_BLINK_PERIOD_TIMER_C 8000 

/*! @brief typedef que define los posibles estados de distancia del sistema*/
typedef enum estados_de_distancia
{
	DISTANCIA_ACEPTABLE,
	DISTANCIA_PRECAUCION,
	DISTANCIA_PELIGRO,

} estado_distancia;

/*! @brief variable de tipo estado_distancia que almacena el valor actual del sistema */
estado_distancia ESTADO_DISTANCIA_ACTUAL;

/*==================[internal data definition]===============================*/
/*! @brief Task handle para la tarea Tomar Distancia */
TaskHandle_t taskTomarDistancia_task_handle = NULL;

/*! @brief Task handle para la tarea Medir Aceleración*/
TaskHandle_t  taskMedirAceleracion_task_handle = NULL;

/*! @brief Task handle para la tarea de manejo del sistema */
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

 /** @fn void FuncTimerMedirAceleracion(void* param)
 * @brief Función invocada por la interrupción del timer A para notificar a la tarea de medición de aceleración.
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerMedirAceleracion(void* param)
{
    vTaskNotifyGiveFromISR(taskMedirAceleracion_task_handle, pdFALSE);    	
}

 /** @fn void FuncTimerManejarSistema(void* param)
 * @brief Función invocada por la interrupción del timer C para notificar a la tarea de manejo del sistema
 * @param param Parámetro de la tarea (no utilizado).
 */
void FuncTimerManejarSistema(void* param)
{
    vTaskNotifyGiveFromISR(taskManejarSistema_task_handle, pdFALSE);    	
}

 /** @fn void inicializarBuzzer()
 * @brief Función que inicializa el buzzer
 */
void inicializarBuzzer()
{
	void GPIOInit(GPIO_6, GPIO_OUTPUT);
}

 /** @fn void detectarEstadoDeDistancia()
 * @brief función que detecta el estado del sistema 
 */
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

 /** @fn void inicializarAcelerometro()
 * @brief función que inicializa el acelerometro
 */
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

 /** @fn void encenderBuzzerPrecaucion()
 * @brief función que enciende el buzzer con una frecuencia de un segundo
 */
void encenderBuzzerPrecaucion()
{
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(DELAY_PRECAUCION / portTICK_PERIOD_MS);
}

 /** @fn void encenderBuzzerPeligro()
 * @brief función que enciende el buzzer con una frecuencia de medio segundo
 */
void encenderBuzzerPeligro()
{
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(DELAY_PELIGRO / portTICK_PERIOD_MS);
}

 /** @fn void apagarBuzzer()
 * @brief función que apaga el buzzer
 */
void apagarBuzzer()
{
	GPIOOff(GPIO_BUZZER);
}

 /** @fn void manejarPerifericos()
 * @brief función que maneja los leds y el buzzer en funcion de la distancia medida (estado)
 */
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

 /** @fn void enviarDatosApp()
 * @brief función que envia los datos a la aplicación bluetooth por el puerto de uart connection
 */
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

/**
 * @fn taskManejarSistema()
 * @brief Tarea que maneja el sistema completo, tanto los perifericos como el envio de datos 
 *
 */
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

/**
 * @fn taskTomarDistancia(void *pvParameter)
 * @brief Tarea que mide la aceleración en todos los ejes y saca la suma escalar
 */
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
	
	//Falto inicializar el HCSR
	HcSr04Init(GPIO_7, GPIO_8); 

	inicializarBuzzer();

	inicializarAcelerometro();

	//Me faltó cambiar el nombre de cada timer :'D

	timer_config_t timer_medir_aceleracion = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TIMER_A,
        .func_p = FuncTimerMedirAceleracion,
        .param_p = NULL
    };

		timer_config_t timer_tomar_distancia = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimerTomarDistancia,
        .param_p = NULL
    };

	timer_config_t timer_sistema= {
        .timer = TIMER_C,
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