/*
 * ldr.h
 *
 *  Created on: 3 nov. 2022
 *      Author: ginos
 */

#ifndef LDR_H_
#define LDR_H_

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#include "stepper_motor.h"
#include "bluetooh.h"

#define PORT_0 (uint8_t) 0
#define CH0 (uint8_t) 0
#define CH1 (uint8_t) 1

void init_ldr();
void enable_ldr();
void disable_ldr();

#endif /* LDR_H_ */
