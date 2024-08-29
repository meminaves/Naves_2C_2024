/*! @mainpage Guia 1 Ejercicio 6
 *
 * @section Descripción
 * Este programa se encarga de mostrar por una pantalla lcd un valor de 32 bits.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1		 	| 	GPIO_20		|
 * | 	D2		 	| 	GPIO_21		|
 * | 	D3		 	| 	GPIO_22		|
 * | 	D4		 	| 	GPIO_23		| 
 * | 	SEL_1	 	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V	 		| 	+5V			|
 * | 	GND		 	| 	GND			|
 * 
 * 
 * @section changelog Changelog
 *
 * |   Fecha    | Descripción                                   |
 * |:----------:|:-----------------------------------------------|
 * | 29/08/24   | Creación de Documentación		                         |
 *
 * @author Maria Emilia Naves (maria.naves@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/**
 * @def gpioConf_t
 * @brief estructura de tipo definida que representa un GPIO, junto a su numero de pin
 * y su direccion
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/**
 * @def vector_pines
 * @brief vector de tipo gpioConf_t que contiene los GPIO que controlan la entrada de datos del LCD
 */
gpioConf_t vector_pines[4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}}; 

/**
 * @def vector_pines_selector
 * @brief vector de tipo gpioConf_t que contiene los GPIO que permiten seleccionar qué lcd se enciende
 */
gpioConf_t vector_pines_selector[3] = {{GPIO_19, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};

/*==================[internal functions declaration]=========================*/
/**
 * @fn int8_t convertirBcdaArray (uint32_t dato, uint8_t digitos, uint8_t * nro_bcd)
 * @brief funcion encargada de descomponer un valor en sus digitos bcd. Los almacena en un array 
 * @param dato valor cuyos digitos quieren obtenerse
 * @param digitos cantidad de digitos en los cuales quiere descomponerse el valor
 * @param nro_bcd puntero al array donde se guardaran los digitos bcd
 */
int8_t  convertirBcdaArray (uint32_t dato, uint8_t digitos, uint8_t * nro_bcd)
{
	for (int i = 0; i < digitos; i++)
	{
		nro_bcd[digitos-1-i] = dato%10;
		dato = dato/10; 
	}
	return 1;
}

/**
 * @fn uint8_t BCDaGPIO(uint8_t digito, gpioConf_t * vector_gpioConf)
 * @brief
 * @param
 */
uint8_t BCDaGPIO(uint8_t digito, gpioConf_t * vector_gpioConf){
	uint8_t mascara = 1;

	//Inicializo cada GPIO con su pin y dir
	for (int i=0; i<4; i++)
	{
		GPIOInit(vector_gpioConf[i].pin, vector_gpioConf[i].dir); 
	}

	//De acuerdo a digito, va muestreando los bits y se van poniendo en alto o bajo según corresponda
	for(int j = 0; j < 4; j++)
	{
		if((digito & mascara) != 0)
		{
			GPIOOn(vector_pines[j].pin);
		}
		else{
			GPIOOff(vector_pines[j].pin);	
		}
		mascara = mascara << 1;
	}

return 1;
}
uint8_t mostrarValorPorDisplay(uint32_t valor, uint8_t cantDigitos, gpioConf_t * p_vector_gpioConf, gpioConf_t * p_vector_gpioConf_selector)
{
	//Primero inicializo el vector selector
	for(int i=0; i<3; i++)
	{
		GPIOInit(p_vector_gpioConf_selector[i].pin, p_vector_gpioConf_selector[i].dir);
	}
	uint8_t digitos_bcd[3];

	convertirBcdaArray(valor, cantDigitos, digitos_bcd); //Separo los digitos de mi valor
	 
	for(int i = 0; i < cantDigitos; i++)
	{
		BCDaGPIO(digitos_bcd[i], p_vector_gpioConf);
		GPIOOn(p_vector_gpioConf_selector[i].pin);
		GPIOOff(p_vector_gpioConf_selector[i].pin);
	}
		 //Llamo a la funcion para que convierta cada digito a gpio (lo muestre en el selector)

	//Despues, deberia ir jugando con las llaves selectoras para mostrar cada digito en la pantalla lcd correspondiente
	return 1; 
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	printf("Hello world!\n");
	mostrarValorPorDisplay(777, 3, vector_pines, vector_pines_selector);
}
/*==================[end of file]============================================*/