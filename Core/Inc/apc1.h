/*
 * apc1.h
 *
 *  Created on: May 18, 2024
 *      Author: Nejc
 */

#ifndef INC_APC1_H_
#define INC_APC1_H_

#include "main.h"
#include <stdint.h>
#include <string.h>

#define APC1_Convert(low, high)			(((uint16_t) low << 8) | high)

#define BUFFER_SIZE						100
#define COMMAND_LENGHT					7
#define RESERVED_OUTPUT_REGISTER		0x20
#define RS0_OUTPUT_REGISTER				0x2A
#define AQI_OUTPUT_REGISTER				0X3A
#define ERROR_OUTPUT_REGISTER			0x3D
#define CHECKSUM_LOW_OUTPUT_REGISTER	62
#define CHECKSUM_HIGH_OUTPUT_REGISTER	63
#define SUM_OF_VALUES_MEA				62
#define ANSWER_FRAME_LENGTH_L			3
#define ANSWER_CMD						4
#define	ANSWER_DATA						5
#define SUM_OF_VALUES_CMD				6
#define CHECKSUM_LOW_CMD_ANSWER			6
#define CHECKSUM_HIGH_CMD_ANSWER		7

enum APC1_Status {
	APC1_OK = 0,
	APC1_ERROR_TOO_MANY_FAN_RESTARTS = 0b00000001,
	APC1_ERROR_FAN_SPEED_LOW = 0b00000010,
	APC1_ERROR_PHOTODIODE = 0b00000100,
	APC1_ERROR_FAN_STOPPED = 0b00001000,
	APC1_ERROR_LASER = 0b00010000,
	APC1_ERROR_VOC = 0b00100000,
	APC1_ERROR_RHT = 0b01000000,
	APC1_ERROR_CRC = 0b10000000,
	APC1_ERROR_CMD
};

enum APC1_Commands {
	APC1_CMD_READ_MODULE_TYPE,
	APC1_CMD_READ_MEA_DATA,
	APC1_CMD_SET_PASSIVE_COMM,
	APC1_CMD_SET_ACTIVE_COMM,
	APC1_CMD_SET_IDLE_MODE,
	APC1_CMD_SET_MEA_MODE,
	APC1_NUM_OF_CMD // number of supported commands
};

struct APC1_Command_Settings {
	uint8_t cmd[COMMAND_LENGHT],
			da_frame_lenght_l,
			da_cmd_or_data,
			da_modeL_or_data,
			response_size;
};

struct APC1_Mea_Data {
	uint16_t pm1_0, pm2_5, pm10,
			 pm1_0_air, pm2_5_air, pm10_air,
			 particles_0_3, particles_0_5, particles_1_0,
			 particles_2_5, particles_5_0, particles_10,
			 TVOC, eCO2, t_comp,
			 rh_comp, t_raw, rh_raw;
	uint8_t aqi;

};

enum APC1_Status APC1_Read_Module_Type(void);
enum APC1_Status APC1_Read_Mea_Data(void);
enum APC1_Status APC1_Set_Idle_Mode(void);
enum APC1_Status APC1_Set_Active_Comm_Mode(void); // device to send 64B structure every second
enum APC1_Status APC1_Set_Mea_Mode(void);
enum APC1_Status APC1_Set_Passive_Comm_Mode(void); // device to send 64B structure on request
uint16_t APC1_Get_PM1_0(void);
uint16_t APC1_Get_PM2_5(void);
uint16_t APC1_Get_PM10(void);
uint16_t APC1_Get_PM1_0_air(void);
uint16_t APC1_Get_PM2_5_air(void);
uint16_t APC1_Get_PM10_air(void);
uint16_t APC1_Get_Particles_GT_0_3(void);
uint16_t APC1_Get_Particles_GT_0_5(void);
uint16_t APC1_Get_Particles_GT_1_0(void);
uint16_t APC1_Get_Particles_GT_2_5(void);
uint16_t APC1_Get_Particles_GT_5_0(void);
uint16_t APC1_Get_Particles_GT_10(void);
uint16_t APC1_Get_TVOC(void);
uint16_t APC1_Get_eCO2(void);
double APC1_Get_T_Comp(void);
double APC1_Get_RH_Comp(void);
double APC1_Get_T_Raw(void);
double APC1_Get_RH_Raw(void);
uint8_t	APC1_Get_AQI(void);
const char *APC1_Get_AQI_String(void);
const char *APC1_Get_Error_String(void);

extern void Error_Handler(void);
extern UART_HandleTypeDef huart1;

#endif /* INC_APC1_H_ */
