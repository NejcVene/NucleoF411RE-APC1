
# ScioSense APC1 library for STM

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Examples](#examples)

  

## Introduction

The **ScioSence APC1 library for STM ** is a library designed to work with the ScioSense APC1 sensor on STM boards. It provides a unified API to read data
and send commands. Tested working on NucleoF411RE and STM32H750B-DK.

## Features
- Easy-to-use API
- Error handling
- Supports the use of different USART interfaces
- Data reception by using interrupts
- Data transmission using blocking mode
- Implemented using HAL

  

## Installation
You can get the library by downloading the whole repository or just the `apc1.c` and `apc1.h` files.

  

## Usage
**Note:** the provided project and example code is configured for **NucleoF411RE** and uses **USART1**.
For other STM boards you have to create a new project and add the library to it - See section 2.

1. Whole repository.
If you downloaded the whole repository, just open the provided `APC1.ioc` file and then build the project.


2. `apc1.c` and `apc1.h`
If you downloaded just these two files (for use on a different STM board), then add them to your project
by putting `apc1.c` into `Src` folder and `apc1.h` into `Inc` folder. Configure a USART interface of your
choice to 9600 baud and 8N1 (8 data bits, no parity, 1 stop bit).


## API Reference

- `enum APC1_Status APC1_Init_Sensor(UART_HandleTypeDef *huart)`: Initialize the APC1 sensor by taking a pointer to the used USART interface.
- `enum APC1_Status APC1_Read_Module_Type(void)`: Receives and processes answer for read module type command.
- `enum APC1_Status APC1_Read_Mea_Data(void)`: Receives and processes answer for measurement command.
This function is to be called when user wishes to get latest measurement from sensor.
- `enum APC1_Status APC1_Set_Idle_Mode(void)`: Sets sensor to idle mode.
- `enum APC1_Status APC1_Set_Active_Comm_Mode(void)`: Sets sensor to active communication mode.
Sensor is to send 64B structure every second.
- `enum APC1_Status APC1_Set_Mea_Mode(void)`: Sets sensor to measurement mode.
Depending on the mode of the sensor (active or passive) there is or isn't a response in the buffer.
- `enum APC1_Status APC1_Set_Passive_Comm_Mode(void)`: Sets sensor to passive communication mode.
Sensor is to send 64B structure on request by using the `APC1_Read_Mea_Data` function.
- `uint16_t APC1_Get_PM1_0(void)`: Get PM1.0 mass concentration.
- `uint16_t APC1_Get_PM2_5(void)`: Get PM2.5 mass concentration.
- `uint16_t APC1_Get_PM10(void)`: Get PM10 mass concentration.
- `uint16_t APC1_Get_PM1_0_air(void)`: Get PM1.0 mass concentration in atmospheric environment.
- `uint16_t APC1_Get_PM2_5_air(void)`: Get PM2.5 mass concentration in atmospheric environment.
- `uint16_t APC1_Get_PM10_air(void)`: Get PM10 mass concentration in atmospheric environment.
- `uint16_t APC1_Get_Particles_GT_0_3(void)`: Get number of particles with diameter > 0.3µm in 0.1L of air.
- `uint16_t APC1_Get_Particles_GT_0_5(void)`: Get number of particles with diameter > 0.5µm in 0.1L of air.
- `uint16_t APC1_Get_Particles_GT_1_0(void)`: Get number of particles with diameter > 1.0µm in 0.1L of air.
- `uint16_t APC1_Get_Particles_GT_2_5(void)`: Get number of particles with diameter > 2.5µm in 0.1L of air.
- `uint16_t APC1_Get_Particles_GT_5_0(void)`: Get number of particles with diameter > 5.0µm in 0.1L of air.
- `uint16_t APC1_Get_Particles_GT_10(void)`: Get number of particles with diameter > 10µm in 0.1L of air.
- `uint16_t APC1_Get_TVOC(void)`: Get TVOC output.
- `uint16_t APC1_Get_eCO2(void)`: Get output in ppm CO2 equivalents.
- `double APC1_Get_T_Comp(void)`: Get compensated temperature in °C.
- `double APC1_Get_RH_Comp(void)`: Get compensated RH in %.
- `double APC1_Get_T_Raw(void)`: Get uncompensated temperature in °C.
- `double APC1_Get_RH_Raw(void)`: Get uncompensated RH in %.
- `uint8_t APC1_Get_AQI(void)`: Get Air Quality Index according to UBA Classification of TVOC value.
- `const char *APC1_Get_AQI_String(void)`: Get Air Quality Index according to UBA Classification of TVOC value as describing string.
- `char *APC1_Get_Error_String(void)`: Get error value as describing string.


## Examples
A simple example is provided in the `main.c` file and should apply to all STM boards if the HAL library is used.
If HAL is not used, then changes have to be made in receiving and transmitting data.