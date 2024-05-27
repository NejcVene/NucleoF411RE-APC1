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

HAL_StatusTypeDef APC1_Send_Receive_Command(UART_HandleTypeDef *huart, struct APC1_Command_Settings setting, int get_response);
enum APC1_Status APC1_Check_Checksum(int limit, int low, int high);
int APC1_Check_For_Error(void);
enum APC1_Status APC1_Check_Command_Answer(struct APC1_Command_Settings setting);
enum APC1_Status APC1_Process_Mea_Data(void);

uint8_t buffer[BUFFER_SIZE];
char error_buffer[BUFFER_SIZE];
volatile int received_response;
struct APC1_Device_Settings dev_settings;
struct APC1_Mea_Data processed_data;
const char *APC1_AQI_Strings[5] = {
		"Good",
		"Fair",
		"Poor",
		"Very poor",
		"Extremely Poor"
};

const char *APC1_Status_Strings[] = {
		"APC1_OK",
		"APC1_ERROR_TOO_MANY_FAN_RESTARTS",
		"APC1_ERROR_FAN_SPEED_LOW",
		"APC1_ERROR_PHOTODIODE",
		"APC1_ERROR_FAN_STOPPED",
		"APC1_ERROR_LASER",
		"APC1_ERROR_VOC",
		"APC1_ERROR_RHT",
		"APC1_ERROR_CRC",
		"APC1_ERROR_CMD"
};

const uint8_t APC1_Error_Masks[] = {
		APC1_OK,
		APC1_ERROR_TOO_MANY_FAN_RESTARTS,
		APC1_ERROR_FAN_SPEED_LOW,
		APC1_ERROR_PHOTODIODE,
		APC1_ERROR_FAN_STOPPED,
		APC1_ERROR_LASER,
		APC1_ERROR_VOC,
		APC1_ERROR_RHT,
		APC1_ERROR_CRC,
		APC1_ERROR_CMD
};

enum APC1_Status APC1_Init_Sensor(UART_HandleTypeDef *huart) {

	if (huart == NULL) {
		return APC1_ERROR_NULL_POINTER;
	}

	dev_settings = (struct APC1_Device_Settings) {
			.sensor_uart = huart,
			.mode = APC1_PASSIVE_MODE,
			.fw_vesion = 0
	};

	return APC1_OK;

}

HAL_StatusTypeDef APC1_Send_Receive_Command(UART_HandleTypeDef *huart, struct APC1_Command_Settings setting, int get_response) {

	HAL_StatusTypeDef status;

	memset(buffer, 0, BUFFER_SIZE);
	if (get_response && (status = HAL_UART_Receive_IT(huart, buffer, setting.response_size)) != HAL_OK) {
		return status;
	}
	if ((status = HAL_UART_Transmit(huart, setting.cmd, COMMAND_LENGHT, 2000)) != HAL_OK) {
		return status;
	}

	return status;

}

enum APC1_Status APC1_Read_Module_Type(void) {

	enum APC1_Status status;

	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_READ_MODULE_TYPE], GET_RESPONSE) != HAL_OK) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}
	while (received_response == 0);
	received_response = 0;

	if ((status = APC1_Check_Checksum(SUM_OF_VALUES_FW, CHECKSUM_LOW_FW, CHECKSUM_HIGH_FW)) != APC1_OK) {
		APC1_Error_Buffer_Append(error_buffer, 8);
		return status;
	}
	dev_settings.fw_vesion = buffer[FW_ANSWER_FW_VERSION];

	return status;

}

enum APC1_Status APC1_Read_Mea_Data(void) {

	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_READ_MEA_DATA], GET_RESPONSE) != HAL_OK) {
		return APC1_ERROR_CMD;
	}
	while (received_response == 0);
	received_response = 0;

	return APC1_Process_Mea_Data();

}

enum APC1_Status APC1_Process_Mea_Data(void) {

	enum APC1_Status errorStat;

	if ((errorStat = APC1_Check_Checksum(SUM_OF_VALUES_MEA, CHECKSUM_LOW_OUTPUT_REGISTER, CHECKSUM_HIGH_OUTPUT_REGISTER)) != APC1_OK) {
		APC1_Error_Buffer_Append(error_buffer, 8);
		return errorStat;
	}

	if (APC1_Check_For_Error()) {
		return APC1_ERROR_CMD; // it does not matter which error we return here, as the correct errors are reported in the error buffer
	}

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
	dev_settings.fw_vesion = buffer[VERSION_OUTPUT_REGISTER];

	return APC1_OK;

}

enum APC1_Status APC1_Check_Checksum(int limit, int low, int high) {

	int sum = 0;
	for (int i = 0; i<limit; i++) {
		sum += buffer[i];
	}

	return (APC1_Convert(buffer[low], buffer[high])) == sum ? APC1_OK : APC1_ERROR_CRC;

}

int APC1_Check_For_Error(void) {

	error_buffer[0] = '\0';
	uint8_t status = buffer[ERROR_OUTPUT_REGISTER];

	if (status == APC1_OK) {
		strcat(error_buffer, APC1_Status_Strings[0]);
		error_buffer[strlen(error_buffer) - 1] = '\0';
	} else {
		for (int i = 0; i<NUM_OF_ERRORS; i++) {
			if (status & APC1_Error_Masks[i]) {
				APC1_Error_Buffer_Append(error_buffer, i);
				strcat(error_buffer, ";");
			}
		}
		error_buffer[strlen(error_buffer) - 1] = '\0';

		return 1;
	}

	return 0;

}

enum APC1_Status APC1_Set_Idle_Mode(void) {

	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_SET_IDLE_MODE], GET_RESPONSE) != HAL_OK) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}
	while (received_response == 0);
	received_response = 0;

	return APC1_Check_Command_Answer(command[APC1_CMD_SET_IDLE_MODE]);

}

enum APC1_Status APC1_Set_Active_Comm_Mode(void) {

	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_SET_ACTIVE_COMM], GET_RESPONSE) != HAL_OK) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}
	while (received_response == 0);
	received_response = 0;
	dev_settings.mode = APC1_ACTIVE_MODE;

	return APC1_Check_Command_Answer(command[APC1_CMD_SET_ACTIVE_COMM]);

}

// TODO: still a mystery what happens in active mode
enum APC1_Status APC1_Set_Mea_Mode(void) {

	// if device mode is ACTIVE, then there is a response, otherwise not
	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_SET_MEA_MODE],
		(dev_settings.mode) ? GET_RESPONSE : NO_RESPONSE) != HAL_OK) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}
	if (dev_settings.mode) {
		while (received_response == 0);
		received_response = 0;
		return APC1_Process_Mea_Data();
	}

	return APC1_OK;

}

enum APC1_Status APC1_Set_Passive_Comm_Mode(void) {

	if (APC1_Send_Receive_Command(dev_settings.sensor_uart, command[APC1_CMD_SET_PASSIVE_COMM], GET_RESPONSE) != HAL_OK) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}
	while (received_response == 0);
	received_response = 0;
	dev_settings.mode = APC1_PASSIVE_MODE;

	return APC1_Check_Command_Answer(command[APC1_CMD_SET_PASSIVE_COMM]);

}

enum APC1_Status APC1_Check_Command_Answer(struct APC1_Command_Settings setting) {

	enum APC1_Status err;

	if (setting.da_frame_lenght_l != buffer[ANSWER_FRAME_LENGTH_L] ||
		setting.da_cmd_or_data != buffer[ANSWER_CMD] ||
		setting.da_modeL_or_data != buffer[ANSWER_DATA]) {
		APC1_Error_Buffer_Append(error_buffer, 9);
		return APC1_ERROR_CMD;
	}

	if ((err = APC1_Check_Checksum(SUM_OF_VALUES_CMD, CHECKSUM_LOW_CMD_ANSWER, CHECKSUM_HIGH_CMD_ANSWER))) {
		APC1_Error_Buffer_Append(error_buffer, 8);
	}

	return err;

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

const char *APC1_Get_AQI_String(void) {

	// if somehow we get a wrong index
	uint8_t index = APC1_Get_AQI();
	return (index <= 5 && index >= 1) ? APC1_AQI_Strings[index - 1] : "Index out of bounds\0";

}

char *APC1_Get_Error_String(void) {

	return error_buffer;

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART1) {
		received_response = 1;
	}

}
