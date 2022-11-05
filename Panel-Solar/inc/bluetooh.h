/*
 * bluetooh.h
 *
 *  Created on: 3 nov. 2022
 *      Author: ginos
 */

#ifndef BLUETOOH_H_
#define BLUETOOH_H_

#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DMA_SIZE 468

void initUART();
void send_message(uint8_t msg[], uint32_t size);
void print_msg(char *msg);

#endif /* BLUETOOH_H_ */
