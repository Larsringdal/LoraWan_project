/*
 * lora.h
 *
 *  Created on: 27 Apr 2022
 *      Author: larsi
 */

#ifndef INC_LORA_H_
#define INC_LORA_H_

#include "main.h"

typedef enum{
	LORA_OK,
	LORA_INVALID_PARAM,
	LORA_NOT_JOINED,
	LORA_NO_FREE_CH,
	LORA_SILENT,
	LORA_BUSY,
	LORA_MAC_PAUSED,
	LORA_INVALID_DATA_LEN,
	LORA_MAC_TX_OK,
	LORA_RX,
	LORA_MAC_ERR,
	LORA_UART_ERR
}lora_response;


typedef struct{
	UART_HandleTypeDef * huart;
	uint8_t joined;
	uint8_t hweui[100];
} lora_instance;

void init_lora_instance(UART_HandleTypeDef * huart, lora_instance * lora);

void lora_clear_buffer(lora_instance * lora);

uint8_t readUart(lora_instance * lora, uint8_t read_buf[]);

// Returns 0 on successful UART communication. Response stored in response_buf
uint8_t lora_sendCmd(lora_instance * lora, uint8_t cmd[], uint8_t response_buf[]);

uint8_t lora_getHweui(lora_instance * lora, uint8_t * dst);

lora_response lora_joinOtaa(lora_instance * lora);

lora_response lora_sendData(lora_instance * lora, uint16_t data);

lora_response lora_sleep(lora_instance * lora, int ms);

#endif /* INC_LORA_H_ */
