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

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
uint8_t BCDaGPIO(uint8_t digito, gpioConf_t * vector_gpioConf){
	uint8_t mascara = 1;
	for (int i=0; i<4; i++)
	{
		GPIOInit(vector_gpioConf[i].pin, vector_gpioConf[i].dir); 
	}
	for(int j = 0; j < 4; j++)
	{
		if((digito&mascara) != 0)
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

void app_main(void){
	printf("Hello world!\n");

	uint8_t digito = 7;

	BCDaGPIO(digito, vector_pines);
}
/*==================[end of file]============================================*/