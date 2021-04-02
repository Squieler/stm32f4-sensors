/*
 * Library Name: 	ADS1115 STM32 Single-Ended, Single-Shot, PGA & Data Rate Enabled ADC Library
 * Written By:		Ahmet Batuhan Günaltay
 * Date Written:	02/04/2021 (DD/MM/YYYY)
 * Last Modified:	02/04/2021 (DD/MM/YYYY)
 * Description:		STM32F4 HAL-Based ADS1115 Library
 * References:
 * 	- https://www.ti.com/lit/gpn/ADS1113 [Datasheet]
 *
 * Copyright (C) 2021 - Ahmet Batuhan Günaltay
 *
	 This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
	 of the GNU General Public Licenseversion 3 as published by the Free Software Foundation.

	 This software library is shared with puplic for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
	 or indirectly by this software, read more about this on the GNU General Public License.
 *
 *  */

/* Definitions */
#define ADS1115_OS (0b1 << 15) // Default

#define ADS1115_MUX_AIN0 (0b100 << 12)		// Analog input 1
#define ADS1115_MUX_AIN1 (0b101 << 12)		// Analog input 2
#define ADS1115_MUX_AIN2 (0b110 << 12)		// Analog input 3
#define ADS1115_MUX_AIN3 (0b111 << 12)		// Analog input 4

#define ADS1115_PGA_TWOTHIRDS 	(0b000 << 9) 		// 2/3x Gain	-- 0.1875 mV by one bit		MAX: +- VDD + 0.3V
#define ADS1115_PGA_ONE			(0b001 << 9) 		// 1x Gain		-- 0.125 mV by one bit		MAX: +- VDD + 0.3V
#define ADS1115_PGA_TWO			(0b010 << 9) 		// 2x Gain		-- 0.0625 mV by one bit		MAX: +- 2.048 V
#define ADS1115_PGA_FOUR		(0b011 << 9) 		// 4x Gain		-- 0.03125 mV by one bit	MAX: +- 1.024 V
#define ADS1115_PGA_EIGHT		(0b100 << 9) 		// 8x Gain		-- 0.015625 mV by one bit	MAX: +- 0.512 V
#define ADS1115_PGA_SIXTEEN		(0b111 << 9) 		// 16x Gain		-- 0.0078125 mV by one bit	MAX: +- 0.256 V

#define ADS1115_MODE (0b1 << 8) // Default

#define ADS1115_DATA_RATE_8		(0b000 << 5)			// 8SPS
#define ADS1115_DATA_RATE_16	(0b001 << 5)			// 16SPS
#define ADS1115_DATA_RATE_32	(0b010 << 5)			// 32SPS
#define ADS1115_DATA_RATE_64	(0b011 << 5)			// 64SPS
#define ADS1115_DATA_RATE_128	(0b100 << 5)			// 128SPS
#define ADS1115_DATA_RATE_250	(0b101 << 5)			// 250SPS
#define ADS1115_DATA_RATE_475	(0b110 << 5)			// 475SPS
#define ADS1115_DATA_RATE_860	(0b111 << 5)			// 860SPS

#define ADS1115_COMP_MODE 	(0b0 << 4) // Default
#define ADS1115_COMP_POL 	(0b0 << 3) // Default
#define ADS1115_COMP_LAT 	(0b0 << 2) // Default
#define ADS1115_COMP_QUE 	(0b11)	   // Default

/* ADS1115 register configurations */
#define ADS1115_CONVER_REG 0x0
#define ADS1115_CONFIG_REG 0x1

/* TIMEOUT */
#define ADS1115_TIMEOUT 50

/* Function prototypes. */
HAL_StatusTypeDef ADS1115_Init(I2C_HandleTypeDef *handler, uint16_t setDataRate, uint16_t setPGA);
HAL_StatusTypeDef ADS1115_readSingleEnded(uint16_t muxPort, float *voltage);
