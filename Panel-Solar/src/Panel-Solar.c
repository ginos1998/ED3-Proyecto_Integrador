/*
===============================================================================
 Name        : FIRMWARE.c
 Author      : Siampichetti Gino
 	 	 	   Maffini Agustin
 	 	 	   Giorda Marcos
 Version     : 0.0.0
 Copyright   : GLP-3.0 License
 Description : Solar Tracker (README.md for more details)
===============================================================================
*/

// librerias de perifericos
#include "LPC17xx.h"

// librerias del proyecto
#include "ldr.h"
#include "bluetooh.h"
#include "keyboard.h"
#include "stepper_motor.h"

int main(void) {

	init_ldr();
	initStepper();

	init_keyboard();
	initUART();


    while(1);

    return 0 ;
}
