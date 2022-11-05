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

#define PORT_0 (uint8_t) 0

#define BOBINA_0_PIN (uint8_t) (6) 	// P0.6
#define BOBINA_1_PIN (uint8_t) (7) 	// P0.7
#define BOBINA_2_PIN (uint8_t) (8)	// P0.8
#define BOBINA_3_PIN (uint8_t) (9)	// P0.9

#define OUTPUT (uint8_t) 1

void initStepper();
void turnAngle(uint32_t angle);
void set_mode(uint8_t mode);
void stopMotor();
void start_motor();

#endif /* STEPPER_MOTOR_H_ */
