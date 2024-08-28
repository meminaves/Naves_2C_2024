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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t vector_pines[4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}}; 
gpioConf_t vector_pines_selector[3] = {{GPIO_19, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};

/*==================[internal functions declaration]=========================*/
mostrarValorPorDisplay(uint32 valor, uint cantDigitos, gpioConf_t * vector_gpioConf, gpioConf_t * vector_gpioConf_selector)
{
	//Primero inicializo el vector selector
	for(int i=0; i<3; i++)
	{
		GPIOInit(vector_gpioConf_selector[i].pin, vector_gpioConf_selector[i].dir);
	}

	//uint8_t digitos_bcd[3]

	//convertirBcdaArray(valor, cantDigitos, digitos_bcd) //Separo los digitos de mi valor

	//BCDaGPIO(digitos_bcd[], vector_gpioConf) //Llamo a la funcion para que convierta cada digito a gpio (lo muestre en el selector)

	//Despues, deberia ir jugando con las llaves selectoras para mostrar cada digito en la pantalla lcd correspondiente
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/