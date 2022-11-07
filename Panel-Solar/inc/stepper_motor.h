/*
 * stepper_motor.h
 *
 *  Created on: 3 nov. 2022
 *      Author: ginos
 */

#ifndef STEPPER_MOTOR_H_
#define STEPPER_MOTOR_H_

#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

#include "keyboard.h"

#define PORT_0 (uint8_t) 0
#define OUTPUT (uint8_t) 1

#define BOBINA_0_PIN (uint8_t) (6) 	// P0.6
#define BOBINA_1_PIN (uint8_t) (7) 	// P0.7
#define BOBINA_2_PIN (uint8_t) (8)	// P0.8
#define BOBINA_3_PIN (uint8_t) (9)	// P0.9

typedef enum{
	ROTATE_COUNTERCLOCKWISE = 0,
	ROTATE_CLOCKWISE = 1,
	STOP_MOTOR = 2
}MOTOR_STATUS;

void initStepper();
void turnAngle(uint32_t angle);
void set_mode(MOTOR_STATUS mode);
void stopMotor();
void start_motor();
void set_calib();
float get_current_angle();

#endif /* STEPPER_MOTOR_H_ */
