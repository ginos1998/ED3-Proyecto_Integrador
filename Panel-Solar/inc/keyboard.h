/*
 * keyboard.h
 *
 *  Created on: 3 nov. 2022
 *      Author: ginos
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#include "stepper_motor.h"
#include "bluetooh.h"
#include "ldr.h"

typedef enum{
	// pines asociados a columnas
	PIN_0 = (uint32_t) 0,
	PIN_1 = (uint32_t) 1,
	PIN_2 = (uint32_t) 2,
	PIN_3 = (uint32_t) 3,
	// pines asociados a filas
	PIN_4 = (uint32_t) 4,
	PIN_5 = (uint32_t) 5,
	PIN_6 = (uint32_t) 6,
	PIN_7 = (uint32_t) 7,
	// pin asociado al sensor 'final de carrera'
	PIN_18 = (uint32_t) 18,
	// pin asociado al LED de la placa
	PIN_22 = (uint32_t) 22
}PINES;

// puerto asociado al teclado
#define PORT_2 (uint8_t) 2
#define PORT_0 (uint8_t) 0
#define OUTPUT (uint8_t) 1
#define INPUT  (uint8_t) 0
#define RISING_EDGE  (uint8_t) 0
#define FALLING_EDGE (uint8_t) 1
#define ROW_PINS	(uint8_t) 0x0F
#define COL_PINS	(uint8_t) 0xF0


void init_keyboard();
uint8_t get_key();


#endif /* KEYBOARD_H_ */
