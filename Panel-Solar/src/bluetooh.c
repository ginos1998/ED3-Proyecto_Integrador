/*
 * bluetooh.c
 *
 *  Created on: 22 oct. 2022
 *      Author: ginos
 */
#include "bluetooh.h"

void config_pin_uart();
void config_uart();
void config_gpdma();

GPDMA_Channel_CFG_Type GPDMA_Struct;

char str_send_package[512];


char UART_array[64];

uint8_t envio = 0;


void initUART(){


	// strcat(str_send_packa	ge, str_msg_incio);
	config_pin_uart();
	config_uart();
	config_gpdma();
	char *str_msg_incio = "\n\r********** PANEL SOLAR **********\n\r"
								"\n\r\n\rPresione D para ingresar al Menu\n\r";
	print_msg(str_msg_incio);
	//uint8_t msg[] = {"***** PANEL SOLAR *****\n\r"
	//				"\n\r\n\rPresione D para ingresar al Menu\n\r"};
	//send_message(msg, sizeof(msg));



}

void config_pin_uart(){
	PINSEL_CFG_Type Pinsel_UART;
	Pinsel_UART.Funcnum = 1;		// Funcion 01: (Tx y Rx)
	Pinsel_UART.OpenDrain = 0;		// OD desactivado
	Pinsel_UART.Pinmode = 0;		// Pull-Up
	Pinsel_UART.Pinnum = 10;			// P0.10 para Tx (Conectar cable Blanco)
	Pinsel_UART.Portnum = 0;		// Puerto 0
	PINSEL_ConfigPin(&Pinsel_UART);
	Pinsel_UART.Pinnum = 11;			// P0.3 para Rx (Conectar cable Verde)
	PINSEL_ConfigPin(&Pinsel_UART);
}

void config_uart(){
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;			// configuraci�n por defecto:
	UART_ConfigStructInit(&UARTConfigStruct);			// inicializa perif�rico
	UART_Init(LPC_UART2, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);	// Inicializa FIFO
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);	// Habilita transmisi�n
	UART_TxCmd(LPC_UART2, ENABLE);
}

void config_gpdma(){
	GPDMA_LLI_Type DMA_LLI_Struct;
	//Prepare DMA link list item structure
	DMA_LLI_Struct.SrcAddr= (uint32_t)str_send_package;
	DMA_LLI_Struct.DstAddr= (uint32_t)&(LPC_UART2->THR);
	DMA_LLI_Struct.NextLLI= (uint32_t)str_send_package;
	DMA_LLI_Struct.Control= DMA_SIZE
							| (0<<18) //source width 8 bit
							| (0<<21) //dest. width 8 bit
							| (1<<26) //source increment
							;
	// GPDMA block section --------------------------------------------
	// Initialize GPDMA controller
	GPDMA_Init();
	// Setup GPDMA channel --------------------------------
	// channel 0
	GPDMA_Struct.ChannelNum = 0;
	// Source memory
	GPDMA_Struct.SrcMemAddr = (uint32_t)(str_send_package);
	// Destination memory - unused
	GPDMA_Struct.DstMemAddr = 0;
	// Transfer size
	GPDMA_Struct.TransferSize = DMA_SIZE;
	// Transfer width - unused
	GPDMA_Struct.TransferWidth = 0;
	// Transfer type
	GPDMA_Struct.TransferType = GPDMA_TRANSFERTYPE_M2P;
	// Source connection - unused
	GPDMA_Struct.SrcConn = 0;
	// Destination connection
	GPDMA_Struct.DstConn = GPDMA_CONN_UART2_Tx;
	// Linker List Item - unused
	GPDMA_Struct.DMALLI = (uint32_t)&DMA_LLI_Struct;
	// Setup channel with given parameter

	//GPDMA_ChannelCmd(0, ENABLE);
	//NVIC_EnableIRQ(DMA_IRQn);
}

void DMA_IRQHandler(void){
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR,0);
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC,0);
	GPDMA_ChannelCmd(0, DISABLE);
	LPC_GPDMACH0->DMACCConfig = 0;
	NVIC_DisableIRQ(DMA_IRQn);

}

void send_message(uint8_t msg[], uint32_t size){
	//UART_TxCmd(LPC_UART2, ENABLE);

	UART_Send(LPC_UART2, msg, size, BLOCKING);

	envio++;
	//UART_TxCmd(LPC_UART2, DISABLE);
}

void print_msg(char *msg){


	strcat(str_send_package, msg);
	GPDMA_Setup(&GPDMA_Struct);
	GPDMA_ChannelCmd(0, ENABLE);
	NVIC_EnableIRQ(DMA_IRQn);

}



