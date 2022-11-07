/*
 * ldr.c
 *
 *  Created on: 22 oct. 2022
 *      Author: ginos
 */
#include "ldr.h"

uint32_t ldr_1_val = 0;
uint32_t ldr_2_val = 0;

uint32_t match_val_ldr = 10;
uint32_t match_val_op = 500;
uint32_t operating_time = 0;

uint32_t total_op_time = 0;

uint32_t operating_mode = 0;

uint32_t operating = 1;

uint32_t cont = 0;

uint8_t i = 0;

int error = 0;

void config_pin();
void config_adc();
void config_timer1_ldr();
void config_timer3_ldr();

void init_ldr(){
	config_pin();
	config_adc();
	config_timer1_ldr();
	config_timer3_ldr();
}

void config_pin(){
	PINSEL_CFG_Type pinsel0;
	pinsel0.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinsel0.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinsel0.Funcnum = PINSEL_FUNC_1;
	pinsel0.Portnum = PINSEL_PORT_0;
	pinsel0.Pinnum = PINSEL_PIN_23;
	PINSEL_ConfigPin(&pinsel0);
	pinsel0.Pinnum = PINSEL_PIN_24;
	PINSEL_ConfigPin(&pinsel0);

}

void config_adc(){
	ADC_Init(LPC_ADC, 200000);

	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);

	ADC_BurstCmd(LPC_ADC, 1);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT10);

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, SET);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, SET);

	NVIC_DisableIRQ(ADC_IRQn);
}

void config_timer1_ldr(){
	TIM_TIMERCFG_Type timer1;
	timer1.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer1.PrescaleValue = 22726;
	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer1);

	TIM_MATCHCFG_Type mat10;
	mat10.StopOnMatch = DISABLE;
	mat10.ResetOnMatch = ENABLE;
	mat10.MatchChannel = 0;
	mat10.IntOnMatch = ENABLE;
	mat10.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	mat10.MatchValue = match_val_ldr;
	TIM_ConfigMatch(LPC_TIM1, &mat10);

	TIM_Cmd(LPC_TIM1, DISABLE);
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
	// NVIC_SetPriority(TIMER1_IRQn, 1);
	NVIC_DisableIRQ(TIMER1_IRQn);
}

void config_timer3_ldr(){
	TIM_TIMERCFG_Type timer3;
	timer3.PrescaleOption = TIM_PRESCALE_USVAL;
	timer3.PrescaleValue = 1000;
	TIM_Init(LPC_TIM3, TIM_TIMER_MODE, &timer3);

	TIM_MATCHCFG_Type mat30;
	mat30.StopOnMatch = DISABLE;
	mat30.ResetOnMatch = ENABLE;
	mat30.MatchChannel = 0;
	mat30.IntOnMatch = ENABLE;
	mat30.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	mat30.MatchValue = match_val_op;
	TIM_ConfigMatch(LPC_TIM3, &mat30);

	TIM_Cmd(LPC_TIM3, DISABLE);
	TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);
	// NVIC_SetPriority(TIMER3_IRQn, 1);
	NVIC_DisableIRQ(TIMER3_IRQn);
}

void ADC_IRQHandler(){
	NVIC_DisableIRQ(ADC_IRQn);

	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, 1) && ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_1, 1)){
		ldr_1_val = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
		ldr_2_val = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
		error = (ldr_2_val - ldr_1_val);

		if(error < 0) error *= -1;

		if(error > 230){
			if(ldr_1_val > ldr_2_val){
				set_mode(1);
			}else if(ldr_1_val < ldr_2_val){
				set_mode(0);
			}
			i=0;
		}
		else set_mode(2);
	}
	enable_ldr();
}

void TIMER1_IRQHandler(){
	operating_time+=10;
	TIM_Cmd(LPC_TIM1, DISABLE);
	TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
	NVIC_DisableIRQ(TIMER1_IRQn);
}

void TIMER3_IRQHandler(){
	NVIC_DisableIRQ(TIMER3_IRQn);
	if(error <= 230){
		i++;
		if(i == 4){
			set_mode(2);
			disable_ldr();
			TIM_Cmd(LPC_TIM3, DISABLE);
			total_op_time = operating_time/1000;
			char str_msg[30];
			sprintf(str_msg, "\n\rTiempo de operacion: %d", total_op_time);
			print_msg(str_msg);
		}
	}

	TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);
	NVIC_EnableIRQ(TIMER3_IRQn);
}

/*
 * x: 67 --> automatic mode with operating time
 * x: 65 --> only automatic mode
 */
void enable_ldr(int x){
	if(x==67){
		operating_time = 0;
		TIM_Cmd(LPC_TIM3, ENABLE);
		NVIC_EnableIRQ(TIMER3_IRQn);
	}
	TIM_Cmd(LPC_TIM1, ENABLE);
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);
}

void disable_ldr(){
	TIM_Cmd(LPC_TIM1, DISABLE);
	NVIC_DisableIRQ(TIMER1_IRQn);
	NVIC_DisableIRQ(ADC_IRQn);
}
