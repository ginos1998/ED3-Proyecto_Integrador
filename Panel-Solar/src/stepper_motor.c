/*
 * stepper_motor.c
 *
 *  Created on: 22 oct. 2022
 *      Author: ginos
 */
#include "stepper_motor.h"

float 	steps_to_turn = 0;		// pasos para girar el motor hasta un cierto angulo

uint32_t 	steps = 0;

uint32_t 	match_val = 1;
uint8_t 	clockwise = 1;		// variable para indicar en que sentido gira el motor
								// 		1: sentido horario
								// 		0: sentido antihorario
uint8_t 	flag=0;				// flag para multiplexar

uint8_t 	state = 0;

uint8_t 	manual_mod = 0;

uint8_t 	calib = 0;

float current_angle = 0;

float prev_angle = 0;

float total_steps = 0;

/*
 * 1000
 * 1100
 * 0100
 * 0110
 * 0010
 * 0011
 * 0001
 * 1001
 */
uint32_t seq_half_step_right [] = {0x40, 0xC0, 0x80, 0x180, 0x100, 0x300, 0x200, 0x240};

uint32_t seq_half_step_left [] = {0x80, 0xC0, 0x40, 0x240, 0x200, 0x300, 0x100, 0x180};

void config_gpio_motor();
void config_timer0();
void turnRight();
void turnLeft();

void initStepper(){
	config_gpio_motor();
	config_timer0();
}

void config_gpio_motor(){
	PINSEL_CFG_Type pinsel;
	pinsel.Funcnum = PINSEL_FUNC_0;
	pinsel.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinsel.Pinmode = PINSEL_PINMODE_PULLDOWN;
	pinsel.Portnum = PINSEL_PORT_0;
	pinsel.Pinnum = PINSEL_PIN_6;
	PINSEL_ConfigPin(&pinsel);
	pinsel.Pinnum = PINSEL_PIN_7;
	PINSEL_ConfigPin(&pinsel);
	pinsel.Pinnum = PINSEL_PIN_8;
	PINSEL_ConfigPin(&pinsel);
	pinsel.Pinnum = PINSEL_PIN_9;
	PINSEL_ConfigPin(&pinsel);

	GPIO_SetDir(PORT_0, (1<<BOBINA_0_PIN), OUTPUT);
	GPIO_SetDir(PORT_0, (1<<BOBINA_1_PIN), OUTPUT);
	GPIO_SetDir(PORT_0, (1<<BOBINA_2_PIN), OUTPUT);
	GPIO_SetDir(PORT_0, (1<<BOBINA_3_PIN), OUTPUT);

	GPIO_ClearValue(PORT_0, (1<<BOBINA_0_PIN));
	GPIO_ClearValue(PORT_0, (1<<BOBINA_1_PIN));
	GPIO_ClearValue(PORT_0, (1<<BOBINA_2_PIN));
	GPIO_ClearValue(PORT_0, (1<<BOBINA_3_PIN));

}

void config_timer0(){
	TIM_TIMERCFG_Type timer0;
	timer0.PrescaleOption = TIM_PRESCALE_USVAL;
	timer0.PrescaleValue = 1000;
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer0);

	TIM_MATCHCFG_Type mat00;
	mat00.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	mat00.IntOnMatch = ENABLE;
	mat00.MatchChannel = 0;
	mat00.MatchValue = match_val;
	mat00.ResetOnMatch = ENABLE;
	mat00.StopOnMatch = DISABLE;
	TIM_ConfigMatch(LPC_TIM0, &mat00);

	TIM_Cmd(LPC_TIM0, DISABLE);
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

	NVIC_DisableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){
	NVIC_DisableIRQ(TIMER0_IRQn);

	if(state == 1){
		if(clockwise == 1 && get_current_angle() >= 0) turnRight();
		else if (clockwise == 0 && get_current_angle() < 60) turnLeft();
	}else if(state == 0){
		if (clockwise == 2) stopMotor();
	}

	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

/*
 * setea el modo de funcionamiento del motor
 * 	mode 0: sentido antihorario
 * 	mode 1: sentido horario
 * 	mode 2: stop
 * 	mode>2 | mode<0: error ==> por defecto, sentido horario
 */
void set_mode(MOTOR_STATUS mode){

	prev_angle = current_angle;
	total_steps = 0;
	clockwise = mode;
	if(state == 0 && mode < 2){
		state = 1;
		start_motor();
	}
	else if (state == 1 && mode == 2) state = 0;
}

void turnRight(){
	LPC_GPIO0->FIOPIN = seq_half_step_right[flag];
	flag++;
	if(flag == 7)	flag = 0;

	if(calib == 1){
			total_steps++;
			current_angle = prev_angle - (total_steps/2048)*180;
		}else{
			current_angle = 0;
		}

	if(manual_mod == 1){
		steps++;
		if(steps >= steps_to_turn){
			steps = 0;
			manual_mod = 0;
			stopMotor();
			menu();
		}

	}
}

void turnLeft(){
	LPC_GPIO0->FIOPIN = seq_half_step_left[flag];
	flag++;
	if(flag == 7)	flag = 0;

	total_steps++;
	current_angle = prev_angle + (total_steps/2048)*180;

	if(manual_mod == 1){
		steps++;
		if(steps >= steps_to_turn){
			steps = 0;
			manual_mod = 0;
			stopMotor();
			menu();
		}
	}

}

void stopMotor(){
	GPIO_ClearValue(PORT_0, 0x3C0);	// P0.<6,7,8,9> en 0
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
	TIM_Cmd(LPC_TIM0, DISABLE);
	NVIC_DisableIRQ(TIMER0_IRQn);
	state = 0;
}

void turnAngle(uint32_t angle){

	float a = (float) angle;
	steps_to_turn = (a/180)*2048;

	manual_mod = 1;

}

void start_motor(){
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}


void set_calib(){
	char *str_msg = "\n\r..Sistema calibrado\n\r";
	print_msg(str_msg);
	calib = 1;
}

float get_current_angle(){
	return current_angle;
}
