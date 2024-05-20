/*
 * apc1.c
 *
 *  Created on: May 18, 2024
 *      Author: Nejc
 */

#include "apc1.h"

struct APC1_Command_Settings command[APC1_NUM_OF_CMD] = {
		{
				// read module type, ID & FW version
				.cmd = {0x42, 0x4D, 0xE9, 0x00, 0x00, 0x01, 0x78},
				.da_frame_lenght_l = 0xFF,
				.da_cmd_or_data = 0xFF,
				.da_modeL_or_data = 0xFF,
				.response_size = 23
		},
		{
				// read measurement data from device
				.cmd = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71},
				.da_frame_lenght_l = 0xFF,
				.da_cmd_or_data = 0xFF,
				.da_modeL_or_data = 0xFF,
				.response_size = 64
		},
		{
				// set passive communication mode
				.cmd = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70},
				.da_frame_lenght_l = 0x04,
				.da_cmd_or_data = 0xE1,
				.da_modeL_or_data = 0x00,
				.response_size = 8
		},
		{
				// set active communication mode
				.cmd = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71},
				.da_frame_lenght_l = 0x04,
				.da_cmd_or_data = 0xE1,
				.da_modeL_or_data = 0x01,
				.response_size = 8
		},
		{
				// communication protocol to set device to idle mode
				.cmd = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73},
				.da_frame_lenght_l = 0x04,
				.da_cmd_or_data = 0xE4,
				.da_modeL_or_data = 0x00,
				.response_size = 8
		},
		{
				// communication protocol to set device to measurement mode
				.cmd = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74},
				.da_frame_lenght_l = 0xFF,
				.da_cmd_or_data = 0xFF,
				.da_modeL_or_data = 0xFF,
				.response_size = 64
		},
};

HAL_StatusTypeDef APC1_Send_Command(UART_HandleTypeDef *huart, uint8_t *command);
HAL_StatusTypeDef APC1_Receive_Response(UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t size);
enum APC1_Status APC1_Check_Checksum(int limit, int low, int high);
enum APC1_Status APC1_Check_For_Error(void);

uint8_t buffer[BUFFER_SIZE] = {0};
volatile int received_response;
struct APC1_Mea_Data processed_data;

HAL_StatusTypeDef APC1_Send_Command(UART_HandleTypeDef *huart, uint8_t *command) {

	return HAL_UART_Transmit(huart, command, COMMAND_LENGHT, 2000);

}

HAL_StatusTypeDef APC1_Receive_Response(UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t size) {

	memset(buffer, 0, BUFFER_SIZE);
	return HAL_UART_Receive_IT(huart, buffer, size);

}


enum APC1_Status APC1_Read_Module_Type(void) {

	if (APC1_Receive_Response(&huart1, buffer, command[APC1_CMD_READ_MODULE_TYPE].response_size) != HAL_OK) {
		Error_Handler();
	}

	if (APC1_Send_Command(&huart1, command[APC1_CMD_READ_MODULE_TYPE].cmd) != HAL_OK) {
		Error_Handler();
	}

	return APC1_OK;

}

enum APC1_Status APC1_Read_Mea_Data(void) {

	if (APC1_Receive_Response(&huart1, buffer, command[APC1_CMD_READ_MEA_DATA].response_size) != HAL_OK) {
		Error_Handler();
	}

	if (APC1_Send_Command(&huart1, command[APC1_CMD_READ_MEA_DATA].cmd) != HAL_OK) {
		Error_Handler();
	}

	while (received_response == 0);

	if (APC1_Check_Checksum(SUM_OF_VALUES, CHECKSUM_LOW_OUTPUT_REGISTER, CHECKSUM_HIGH_OUTPUT_REGISTER) == APC1_ERROR_CRC) {
		return APC1_ERROR_CRC;
	}

	// if (APC1_Check_For_Error())

	uint16_t index = 4, i = 0;
	uint16_t *struct_member = &processed_data.pm1_0;
	while (index < RS0_OUTPUT_REGISTER) {
		if (index == RESERVED_OUTPUT_REGISTER) { // skip reserved two bytes in output data at address 0x20
			index += 2;
		}
		struct_member[i++] = APC1_Convert(buffer[index], buffer[index + 1]);
		index += 2;
	}
	processed_data.aqi = buffer[AQI_OUTPUT_REGISTER];

	return APC1_OK;

}

enum APC1_Status APC1_Check_Checksum(int limit, int low, int high) {

	int sum = 0;
	for (int i = 0; i<limit; i++) {
		sum += buffer[i];
	}

	return (APC1_Convert(buffer[low], buffer[high])) == sum ? APC1_OK : APC1_ERROR_CRC;

}

enum APC1_Status APC1_Check_For_Error(void) {

	/*
	switch (buffer[61]) {
		case value:

			break;
		default:
			return APC1_OK;
			break;
	}
	*/

	return APC1_OK;

}

uint16_t APC1_Get_PM1_0(void) {

	return processed_data.pm1_0;

}

uint16_t APC1_Get_PM2_5(void) {

	return processed_data.pm2_5;

}

uint16_t APC1_Get_PM10(void) {

	return processed_data.pm10;

}

uint16_t APC1_Get_PM1_0_air(void) {

	return processed_data.pm1_0_air;

}

uint16_t APC1_Get_PM2_5_air(void) {

	return processed_data.pm2_5_air;

}

uint16_t APC1_Get_PM10_air(void) {

	return processed_data.pm10_air;

}

uint16_t APC1_Get_Particles_GT_0_3(void) {

	return processed_data.particles_0_3;

}

uint16_t APC1_Get_Particles_GT_0_5(void) {

	return processed_data.particles_0_5 - processed_data.particles_0_3;

}

uint16_t APC1_Get_Particles_GT_1_0(void) {

	return processed_data.particles_1_0 - processed_data.particles_0_3;

}

uint16_t APC1_Get_Particles_GT_2_5(void) {

	return processed_data.particles_2_5 - processed_data.particles_0_3;

}

uint16_t APC1_Get_Particles_GT_5_0(void) {

	return processed_data.particles_5_0 - processed_data.particles_0_3;

}

uint16_t APC1_Get_Particles_GT_10(void) {

	return processed_data.particles_10 - processed_data.particles_0_3;

}

uint16_t APC1_Get_TVOC(void) {

	return processed_data.TVOC;

}

uint16_t APC1_Get_eCO2(void) {

	return processed_data.eCO2;

}

double APC1_Get_T_Comp(void) {

	return processed_data.t_comp * 0.1f;

}

double APC1_Get_RH_Comp(void) {

	return processed_data.rh_comp * 0.1f;

}

double APC1_Get_T_Raw(void) {

	return processed_data.t_raw * 0.1f;

}

double APC1_Get_RH_Raw(void) {

	return processed_data.rh_raw * 0.1f;

}

uint8_t	APC1_Get_AQI(void) {

	return processed_data.aqi;

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART1) {
		received_response = 1;
	}

}
