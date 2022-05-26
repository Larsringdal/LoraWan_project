/*
 * lora.c
 *
 *  Created on: 27 Apr 2022
 *      Author: larsi
 */
#include "lora.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t lora_sendCmd(lora_instance * lora, uint8_t cmd[], uint8_t response_buf[]){
	HAL_UART_Transmit(lora->huart, cmd, strlen((char*)cmd), 100);
	uint8_t ret = readUart(lora, response_buf);
	return ret;
}

uint8_t readUart(lora_instance * lora, uint8_t read_buf[]){
	uint8_t c[2] = {0};
	uint8_t idx = 0;

	HAL_StatusTypeDef res = HAL_TIMEOUT;

	while(res != HAL_OK){
		res = HAL_UART_Receive(lora->huart, c, 1, 100);
	}

	while (res != HAL_TIMEOUT){
		if (c[0] != '\r' && c[0] != '\n' && res == HAL_OK){
			read_buf[idx++] = c[0];
		}
		res = HAL_UART_Receive(lora->huart, c, 1, 200);
	}
	return 0;
}

// Clears RX line of lora UART
void lora_clear_buffer(lora_instance * lora){
	uint8_t c[2] = {0};
	HAL_StatusTypeDef ret = HAL_OK;
	while (ret != HAL_TIMEOUT)
		ret = HAL_UART_Receive(lora->huart, c, 1, 100);
}

void init_lora_instance(UART_HandleTypeDef * huart, lora_instance * lora){
	lora->huart = huart;
	lora->joined = 0;
	memset(lora->hweui, 0, 100);
	lora_getHweui(lora, lora->hweui);
	lora_clear_buffer(lora);
}

uint8_t lora_getHweui(lora_instance * lora, uint8_t  * dst){
	uint8_t cmd[] = "sys get hweui\r\n";

	uint8_t uartFail = lora_sendCmd(lora, cmd, dst);
	return uartFail;
}

// Attemps to join lora OTAA, returns LORA_OK on success, error code on failure (see lora_response)
lora_response lora_joinOtaa(lora_instance * lora){
	uint8_t cmd[] = "mac join otaa\r\n";
	uint8_t buf[100] = {0};
	HAL_StatusTypeDef ret_uart = HAL_TIMEOUT;

	ret_uart = HAL_UART_Transmit(lora->huart, cmd, strlen((char*)cmd), 100);
	if (ret_uart != HAL_OK) return LORA_UART_ERR;

	readUart(lora, buf);

	if (strcmp((char*)buf, "ok") == 0); // Move to next code block, else return error code
	else if (strcmp((char*)buf, "invalid_param")  == 0) return LORA_INVALID_PARAM;
	else if (strcmp((char*)buf, "busy")  == 0) return LORA_BUSY;
	else if (strcmp((char*)buf, "no_free_ch")  == 0) return LORA_NO_FREE_CH;
	else return LORA_UART_ERR;

	memset(buf, 0, 100);

	readUart(lora, buf);
	if (strcmp((char*)buf, "accepted") == 0){
		return LORA_OK;
		lora->joined = 1;
	}else{
		return LORA_NOT_JOINED;
	}
}

// Puts lora in sleep mode for specified ms
lora_response lora_sleep(lora_instance * lora, int ms){
	if (ms < 100 || ms > 4294967296) return LORA_INVALID_PARAM;

	uint8_t buf[50] = {0};
	uint8_t dst[30] = {0};

	sprintf((char*)buf, "sys sleep %i\r\n", ms);
	HAL_UART_Transmit(lora->huart, buf, strlen((char*)buf), 100);

	readUart(lora, dst);

	if (strcmp((char*)dst, "ok") == 0) return LORA_OK;
	else if (strcmp((char*)dst, "invalid_param") == 0) return LORA_INVALID_PARAM;
	else return LORA_UART_ERR;
}

// Sends up to 16 byte of data OTAA, returns LORA_MAC_TX_OK on success
lora_response lora_sendData(lora_instance * lora, uint16_t data){
	if (lora->joined != 1) return LORA_NOT_JOINED;

	uint8_t buf[100] = {0};
	uint8_t dst[50] = {0};

	sprintf((char*)buf, "mac tx uncnf 1 %X\r\n", data);
	HAL_UART_Transmit(lora->huart, buf, strlen((char*)buf), 100);

	readUart(lora, dst);

	if (strcmp((char*)dst, "ok") == 0);
	else if (strcmp((char*)dst, "invalid_param") == 0) return LORA_INVALID_PARAM;
	else if (strcmp((char*)dst, "not_joined") == 0) return LORA_NOT_JOINED;
	else if (strcmp((char*)dst, "no_free_ch") == 0) return LORA_NO_FREE_CH;
	else if (strcmp((char*)dst, "busy") == 0) return LORA_BUSY;
	else return LORA_UART_ERR;

	memset(dst, 0, 50);

	readUart(lora, dst);
	if (strcmp((char*)dst, "mac_tx_ok") == 0) return LORA_MAC_TX_OK;
	else if (strcmp((char*)dst, "mac_err") == 0) return LORA_MAC_ERR;
	else if (strcmp((char*)dst, "invalid_data_len") == 0) return LORA_INVALID_DATA_LEN;

	return LORA_UART_ERR;
}



