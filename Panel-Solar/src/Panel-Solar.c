/*
===============================================================================
 Name        : FIRMWARE.c
 Author      : Siampichetti Gino
 	 	 	   Maffini Agustin
 	 	 	   Giorda Marcos
 Version     : 1.0.0
 Copyright   : GLP-3.0 License
 Description : Solar Tracker (README.md for more details)
===============================================================================
*/

// librerias de perifericos y LPC
#include "LPC17xx.h"

// librerias del proyecto
#include "ldr.h"
#include "bluetooh.h"
#include "keyboard.h"
#include "stepper_motor.h"

/*
 * @brief Funcion principal que configura los perifericos
 * 			asociados al proyecto y queda a la espera del usuario
 * 			que interactue con el teclado
 *
 * @param none
 * @return 0 (EXIT SCUCCESS)
 */
int main() {

	init_ldr();
	initStepper();
	init_keyboard();
	initUART();

    while(1);

    return 0 ;
}
