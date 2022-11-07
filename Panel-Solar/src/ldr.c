/*
 * @file 	ldr.c
 * @brief	Maneja el sensado de los LDR
 * @version	1.0.0
 * @date	Created on: 22 oct. 2022
 * @author	Grupo 4
 *
 */
#include "ldr.h"

typedef enum{
	MATCH_TIM1 = 10,
	MATCH_TIM3 = 500
}TIMER_MR;

typedef enum{
	PRESCALE_TIM1 = 22726,
	PRESCALE_TIM3 = 1000
}TIMER_PS;

struct{
	TIM_TIMERCFG_Type timer_1;
	TIM_MATCHCFG_Type mat_10;

	TIM_TIMERCFG_Type timer_3;
	TIM_MATCHCFG_Type mat_30;
}TIMER;

struct{
	uint32_t ldr_1_val;
	uint32_t ldr_2_val;
	int error;
}LDR_VALUES;

struct{
	uint32_t operating_time;
	float total_op_time;
}SYS_TIMES;

struct{
	uint8_t n_still;
	uint8_t error_threshold;
	uint8_t op_mode;

}SYS_VAR;

void config_pin();
void config_adc();
void config_timer1_ldr();
void config_timer3_ldr();

void init_ldr(){
	config_pin();
	config_timer1_ldr();
	config_timer3_ldr();
	config_adc();


	LDR_VALUES.ldr_1_val = 0;
	LDR_VALUES.ldr_2_val = 0;
	LDR_VALUES.error = 0;

	SYS_TIMES.operating_time = 0;
	SYS_TIMES.total_op_time = 0;

	SYS_VAR.error_threshold = 220;
	SYS_VAR.n_still = 0;
	SYS_VAR.op_mode = 67;

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
	ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, SET);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, SET);

	NVIC_DisableIRQ(ADC_IRQn);
}

void config_timer1_ldr(){

	TIMER.timer_1.PrescaleOption = TIM_PRESCALE_TICKVAL;
	TIMER.timer_1.PrescaleValue = PRESCALE_TIM1;
	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TIMER.timer_1);

	TIMER.mat_10.StopOnMatch = DISABLE;
	TIMER.mat_10.ResetOnMatch = ENABLE;
	TIMER.mat_10.MatchChannel = 0;
	TIMER.mat_10.IntOnMatch = ENABLE;
	TIMER.mat_10.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIMER.mat_10.MatchValue = MATCH_TIM1;
	TIM_ConfigMatch(LPC_TIM1, &TIMER.mat_10);

	TIM_Cmd(LPC_TIM1, DISABLE);
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
	NVIC_DisableIRQ(TIMER1_IRQn);
}

void config_timer3_ldr(){

	TIMER.timer_3.PrescaleOption = TIM_PRESCALE_USVAL;
	TIMER.timer_3.PrescaleValue = PRESCALE_TIM3;
	TIM_Init(LPC_TIM3, TIM_TIMER_MODE, &TIMER.timer_3);

	TIMER.mat_30.StopOnMatch = DISABLE;
	TIMER.mat_30.ResetOnMatch = ENABLE;
	TIMER.mat_30.MatchChannel = 0;
	TIMER.mat_30.IntOnMatch = ENABLE;
	TIMER.mat_30.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIMER.mat_30.MatchValue = MATCH_TIM3;
	TIM_ConfigMatch(LPC_TIM3, &TIMER.mat_30);
	TIM_Cmd(LPC_TIM3, DISABLE);
	TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);
	NVIC_DisableIRQ(TIMER3_IRQn);
}

void ADC_IRQHandler(){

	NVIC_DisableIRQ(ADC_IRQn);

	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, 1) && ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_1, 1)){
		LDR_VALUES.ldr_1_val = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
		LDR_VALUES.ldr_2_val = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
		LDR_VALUES.error = (LDR_VALUES.ldr_2_val - LDR_VALUES.ldr_1_val);

		if(LDR_VALUES.error < 0) LDR_VALUES.error *= -1;

		if(LDR_VALUES.error > SYS_VAR.error_threshold){
			if(LDR_VALUES.ldr_1_val > LDR_VALUES.ldr_2_val){
				set_mode(ROTATE_CLOCKWISE);
			}else if(LDR_VALUES.ldr_1_val < LDR_VALUES.ldr_2_val){
				set_mode(ROTATE_COUNTERCLOCKWISE);
			}
			SYS_VAR.n_still = 0;
		}
		else set_mode(STOP_MOTOR);
	}

	enable_ldr(1);
}

void TIMER1_IRQHandler(){
	SYS_TIMES.operating_time+=10;
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
	TIM_Cmd(LPC_TIM1, DISABLE);
	NVIC_DisableIRQ(TIMER1_IRQn);
}

void TIMER3_IRQHandler(){
	NVIC_DisableIRQ(TIMER3_IRQn);
	if(LDR_VALUES.error <= SYS_VAR.error_threshold){
		SYS_VAR.n_still++;
		if(SYS_VAR.n_still == 4){
			SYS_VAR.n_still = 0;
			set_mode(STOP_MOTOR);
			disable_ldr();
			TIM_Cmd(LPC_TIM3, DISABLE);
			SYS_TIMES.total_op_time = (float)SYS_TIMES.operating_time/1000 - 1.5;
			char str_msg[40];
			sprintf(str_msg, "\n\rTiempo de operacion: %.3f [s]", SYS_TIMES.total_op_time);
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
void enable_ldr(int mode){
	if(mode == SYS_VAR.op_mode){
		SYS_TIMES.operating_time = 0;
		TIM_Cmd(LPC_TIM3, ENABLE);
		NVIC_EnableIRQ(TIMER3_IRQn);
	}
	TIM_Cmd(LPC_TIM1, ENABLE);
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);

}

void disable_ldr(){
	TIM_Cmd(LPC_TIM1, DISABLE);
	NVIC_DisableIRQ(TIMER1_IRQn);
	NVIC_DisableIRQ(ADC_IRQn);
}
