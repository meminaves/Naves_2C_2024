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

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

int8_t  convertirBcdaArray (uint32_t dato, uint8_t digitos, uint8_t * nro_bcd);

int8_t  convertirBcdaArray (uint32_t dato, uint8_t digitos, uint8_t * nro_bcd)
{
	for (int i = 0; i < digitos; i++)
	{
		nro_bcd[i] = dato%10;
		dato = dato/10; 
	}
	return 1;
}


void app_main(void)
{
	printf("Hello world!\n");
	uint8_t digitos = 3;
	uint32_t dato = 123;
	uint8_t nro_bcd[3];
	
	convertirBcdaArray(dato, digitos, nro_bcd);
	printf("Dato 1: %d\n", nro_bcd[0]);
	printf("Dato 2: %d\n", nro_bcd[1]);
	printf("Dato 3: %d\n", nro_bcd[2]);
}
/*==================[end of file]============================================*/


