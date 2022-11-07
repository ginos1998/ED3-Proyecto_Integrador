/*
 * @file 	bluetooh.c
 * @brief	Configura DMA para ulitiar en comunicacion UART via bluetooth
 * @version	1.0.0
 * @date	Created on: 22 oct. 2022
 * @author	Grupo 4
 *
 */
#include "bluetooh.h"

// funciones 'privadas'
void config_pin_uart();
void config_uart();
void config_gpdma();

// estructura para reciclar GPDMA
GPDMA_Channel_CFG_Type GPDMA_Struct;
// buffer asociado a src de DMA para enviar mensajes por UART
char str_send_package[512];

/*
 * @brief 	Inicia configuraciones para comunicacion UART
 *
 * @param 	none
 *
 * @return 	none
 */
void initUART(){
	config_pin_uart();
	config_uart();
	config_gpdma();
	char *str_msg_incio = "\n\r******* SEGUIDOR SOLAR *******\n\r"
							"\n\r\n\rPresione D para comenzar."
							"\n\rEspere a que el sistema se calibre."
							"\n\rLuego, ingresara al Menu\n\r";
	print_msg(str_msg_incio);
}

/*
 * @brief 	Configuracion de pines para comunicacion UART
 *
 * @param 	none
 *
 * @return 	none
 */
void config_pin_uart(){
	PINSEL_CFG_Type Pinsel_UART;
	Pinsel_UART.Funcnum = PINSEL_FUNC_1;			// Funcion 01: (Tx y Rx)
	Pinsel_UART.OpenDrain = PINSEL_PINMODE_NORMAL;
	Pinsel_UART.Pinmode = PINSEL_PINMODE_PULLUP;
	Pinsel_UART.Portnum = PINSEL_PORT_0;			// Puerto 0
	Pinsel_UART.Pinnum = PINSEL_PIN_10;				// P0.10 para Tx
	PINSEL_ConfigPin(&Pinsel_UART);
	Pinsel_UART.Pinnum = PINSEL_PIN_3;						// P0.11 para Rx
	PINSEL_ConfigPin(&Pinsel_UART);
}

/*
 * @brief 	Configuracion inicial para comunicacion UART
 * @param 	none
 * @return 	none
 */
void config_uart(){
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;			// configuracion por defecto
	UART_ConfigStructInit(&UARTConfigStruct);
	UART_Init(LPC_UART2, &UARTConfigStruct);			// inicializa periferico UART2
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);	// Inicializa FIFO
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);	// Habilita transmision
	UART_TxCmd(LPC_UART2, ENABLE);						// Habilita transmision (Tx) por UART2
}

/*
 * @brief 	Configuracion inicial para General Purpose DMA
 *
 * @param 	none
 *
 * @return 	none
 */
void config_gpdma(){
	GPDMA_LLI_Type DMA_LLI_Struct;
	//Prepare DMA link list item structure
	DMA_LLI_Struct.SrcAddr= (uint32_t)str_send_package;
	DMA_LLI_Struct.DstAddr= (uint32_t)&(LPC_UART2->THR);
	DMA_LLI_Struct.NextLLI= (uint32_t)str_send_package;
	DMA_LLI_Struct.Control= DMA_SIZE
							| (0<<18) 	//source width 8 bit
							| (0<<21) 	//dest. width 8 bit
							| (1<<26) 	//source increment
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
}

/*
 * @brief 	Limpia bandera de interrupcion de DMA,
 * 			deshabilita canal 0,
 * 			deshabilita controlador DMA
 * 			deshabilita NVIC para DMA
 * 			limpio el buffer de transmision
 *
 * @param 	none
 *
 * @return 	none
 */
void DMA_IRQHandler(void){
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR,0);
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC,0);
	GPDMA_ChannelCmd(0, DISABLE);
	LPC_GPDMACH0->DMACCConfig = 0;
	NVIC_DisableIRQ(DMA_IRQn);
	memset(str_send_package, 0, 512);
}

/*
 * @brief 		Wrapper para enviar mensaje por UART (sin DMA)
 *
 * @param[in] 	msg[] 	Mensaje a enviar
 * @param[in] 	size 	tamaño (bytes) del mensaje a enviar
 *
 * @return 	none
 */
void send_message(uint8_t msg[], uint32_t size){
	UART_Send(LPC_UART2, msg, size, BLOCKING);
}

/*
 * @brief 		Concatena el mensaje a enviar al buffer asociado al src de DMA
 * 				Resetea configuracion de GPDMA y habilita la interrupcion para el periférico
 * @param[in] 	*msg 	Mensaje a enviar
 *
 * @return 		none
 */
void print_msg(char *msg){
	strcat(str_send_package, msg);
	GPDMA_Setup(&GPDMA_Struct);
	GPDMA_ChannelCmd(0, ENABLE);
	NVIC_EnableIRQ(DMA_IRQn);

}



