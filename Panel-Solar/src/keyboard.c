/*
 * keyboard.c
 *
 *  Created on: 22 oct. 2022
 *      Author: ginos
 */
#include "keyboard.h"

uint8_t col_val = 0;

int keyboard[4][4] = {{1, 2, 3, 65},
						{4, 5, 6, 66},
						{7, 8, 9, 67},
						{42, 0, 35, 68}};

uint8_t interrupted = 0;

int key = 0;

uint8_t global_state = 0;

uint32_t angle_n;

uint8_t manual = 0;

uint8_t calibrated = 0;


void config_pinsel();
void config_gpio_keyboard();
void config_timer2();
void read_keyboard();
void menu();
void calibrate();

void manual_mode();
void automatic_mode();
void automatic_op_mode();
void get_time();

void init_keyboard(){
	config_pinsel();
	config_gpio_keyboard();
	config_timer2();

	NVIC_EnableIRQ(TIMER2_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);
}

/*
 * CONFIGURACION DE PINES
 *
 * P2 para conectar el teclado 4x4
 * Pines P2.[7:0] como GPIO asociados a pull-down
 *
 * P0.18 con pull-down asociada y funcion GPIO
 */
void config_pinsel(){
	PINSEL_CFG_Type pinsel;
	pinsel.Funcnum = PINSEL_FUNC_0;
	pinsel.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinsel.Pinmode = PINSEL_PINMODE_PULLDOWN;
	pinsel.Portnum = PINSEL_PORT_2;

	for(uint8_t pin = PIN_0; pin<=PIN_7; pin++){
		pinsel.Pinnum = pin;
		PINSEL_ConfigPin(&pinsel);
	}

	pinsel.Portnum = PINSEL_PORT_0;
	pinsel.Pinnum = PIN_18;
	PINSEL_ConfigPin(&pinsel);
}

/*
 * 	CONGIGURAION DE PINES GPIO
 *
 * P2.[7:4] ENTRADAS GPIO por INTERRUPCION FLANCO SUBIDA con PULLDOWN
 * P2.[3:0] SALIDAS GPIO con PULLDOWN
 * P0.22 como salida (LED en la placa)
 *
 */
void config_gpio_keyboard(){
	GPIO_SetDir(PORT_0, (1<<PIN_22), OUTPUT);
	GPIO_ClearValue(PORT_0, (1<<PIN_22));

	GPIO_SetDir(PORT_0, (1<<PIN_18), INPUT);
	GPIO_IntCmd(PORT_0, (1<<PIN_18), FALLING_EDGE);
	GPIO_ClearInt(PORT_0, (1<<PIN_18));

	GPIO_SetDir(PORT_2, ROW_PINS, INPUT);	// P2.[3:0] como entradas
	GPIO_SetDir(PORT_2, COL_PINS, OUTPUT);	// P2.[7:4] como salidas

	GPIO_IntCmd(PORT_2, ROW_PINS, RISING_EDGE);
	GPIO_ClearInt(PORT_2, ROW_PINS);

}


/*
 * TIMER2 configurado para interrumpir cuando haya match
 * ps = 1000 (us), match = 1
 * tiempo de interrumpcion: t = ps*match = 1 [ms]
 */
void config_timer2(){
	TIM_TIMERCFG_Type timer2;
	timer2.PrescaleOption = TIM_PRESCALE_USVAL;
	timer2.PrescaleValue = 1000;
	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer2);

	TIM_MATCHCFG_Type mat20;
	mat20.StopOnMatch = DISABLE;
	mat20.ResetOnMatch = ENABLE;
	mat20.MatchChannel = 0;
	mat20.IntOnMatch = ENABLE;
	mat20.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	mat20.MatchValue = 1;
	TIM_ConfigMatch(LPC_TIM2, &mat20);

	TIM_Cmd(LPC_TIM2, ENABLE);
	TIM_ClearIntPending(LPC_TIM2, TIM_MR2_INT);
}

/*
 * Multiplexa pines P2.[7:4] cada 1 ms entre pin y pin
 */

void TIMER2_IRQHandler(){

	GPIO_ClearValue(PORT_2, COL_PINS);

	if(~(interrupted)){	// si no esta atendiendo una interrupcion, multiplex

		switch(col_val){
				case 0:
					col_val++;
					GPIO_SetValue(PORT_2, (1<<PIN_5));
					break;
				case 1:
					col_val++;
					GPIO_SetValue(PORT_2, (1<<PIN_6));
					break;
				case 2:
					col_val++;
					GPIO_SetValue(PORT_2, (1<<PIN_7));
					break;
				case 3:
					col_val = 0;
					GPIO_SetValue(PORT_2, (1<<PIN_4));
					break;
				default:
					break;
			}
	}
	TIM_ClearIntPending(LPC_TIM2, TIM_MR2_INT);
}

/*
 * Servicio a la interrumpcion para interrupciones por EINT3 (GPIO)
 * - Cada vez que se interrumpe, el LED asociado al pin 22 hace un toggle
 * - Se pueden dar dos casos principales:
 * 		~ Puede interrumpir por PIN_18 (sensor de calibracion).
 * 		~ Puede interrumpor por ROW_PINS del teclado
 * 	En cualquier caso, el bucle for es un simple delay para eliminar rebotes
 */

void EINT3_IRQHandler(){
	NVIC_DisableIRQ(EINT3_IRQn);
	NVIC_DisableIRQ(TIMER2_IRQn);
	TIM_ClearIntPending(LPC_TIM2, TIM_MR2_INT);
	interrupted = 1;

	for(uint32_t i=0; i<=5000000; i++);

	if(LPC_GPIO0->FIOPIN & (1<<22)) GPIO_ClearValue(0,(1<<22));
	else GPIO_SetValue(0,(1<<22));

	read_keyboard();

	if(GPIO_GetIntStatus(PORT_0, PIN_18, FALLING_EDGE)){
		set_calib();
		global_state = 1;
		calibrated = 1;
		set_mode(STOP_MOTOR);
		GPIO_ClearInt(PORT_0, PIN_18);
		LPC_GPIOINT->IO0IntEnF &= ~(1<<PIN_18);
	}
	if(key == LETTER_D && global_state == 0){
		calibrate();
	}

	if(global_state == 1){

		if(calibrated == 1){

			if(manual == 1) manual_mode();
			if(key == LETTER_A) automatic_mode();
			else if(key == LETTER_B) manual_mode();
			else if(key == LETTER_C) automatic_op_mode();
			else if(key == LETTER_D) menu();
		}
	}

	interrupted = 0;
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(TIMER2_IRQn);

}

void read_keyboard(){

	if(GPIO_GetIntStatus(PORT_2, PIN_0, RISING_EDGE)){
		key = keyboard[0][col_val];
		GPIO_ClearInt(PORT_2, (1<<PIN_0));
	}
	else if(GPIO_GetIntStatus(PORT_2, PIN_1, RISING_EDGE)){
		key = keyboard[1][col_val];
		GPIO_ClearInt(PORT_2, (1<<PIN_1));
	}
	else if(GPIO_GetIntStatus(PORT_2, PIN_2, RISING_EDGE)){
		key = keyboard[2][col_val];
		GPIO_ClearInt(PORT_2, (1<<PIN_2));
	}
	else if(GPIO_GetIntStatus(PORT_2, PIN_3, RISING_EDGE)){
		key = keyboard[3][col_val];
		GPIO_ClearInt(PORT_2, PIN_3);
	}

	GPIO_ClearInt(PORT_2, ROW_PINS);

}

uint8_t get_key(){
	return key;
}

void menu(){
	disable_ldr();
	set_mode(STOP_MOTOR);
	char *str_msg = "\n\r***** MENU *****\n\r"
						"Seleccione modo: \n\r"
						"A:\t Automatico \n\r"
						"B:\t Manual \n\r"
						"C:\t Automatico con temporizado\n\r";
	print_msg(str_msg);
}


void manual_mode(){
	static uint8_t cont = 0;
	if(manual == 0){
		char *str_msg = "\n\rModo Manual seleccionado\n\r"
						"\n\rIngrese angulo de giro: (2 digitos)";
		print_msg(str_msg);
		manual = 1;
	}

	if(key < 10 && cont < 3){
		if(cont == 1) angle_n = key*10;
		if(cont == 2){
			angle_n += key;
			char num[84];
			sprintf(num,"\n\rAngulo ingresado: %d??"
						"\n\r'*': Antihorario | '#' Horario", angle_n);
			print_msg(num);
		}
	}

	if(cont == 3){
		turnAngle(angle_n);
		if(key == NUMERAL && (get_current_angle() - angle_n) > 0)
			set_mode(ROTATE_CLOCKWISE);
		else if(key == ASTERISK && (get_current_angle() + angle_n) < 60)
			set_mode(ROTATE_COUNTERCLOCKWISE);
		else{
			char num[84];
			sprintf(num,"\n\rNo se puede girar %d?? "
						"en el sentido indicado", angle_n);
			print_msg(num);
		}
	}

	cont++;
	if(cont == 4){
		cont = 0;
		manual = 0;
		angle_n = 0;
	}
}
void automatic_mode(){

	char *str_msg = "\n\rModo Automatico seleccionado\n\r";
	print_msg(str_msg);

	enable_ldr(key);
	start_motor();
}

void calibrate(){
	char *str_msg = "\n\rCalibrando..\n\r";
	print_msg(str_msg);
	set_mode(ROTATE_CLOCKWISE);
}

void automatic_op_mode(){
	char *str_msg = "\n\rModo Automatico y temporizador seleccionado\n\r";
	print_msg(str_msg);

	enable_ldr(key);
	start_motor();
}


