//
// Created by marijn on 1/12/23.
//

/* //
 * // Pinout
 * //
 *
 * // Diagram
 * 			  ______
 *		A0	-| *	|-	Vcc
 *		A1	-|		|-	WP
 *		A2	-|		|-	SCL
 *		Vss	-|______|-	SDA
 *
 * // Pins
 *	Ax:		I2C address pins
 *	Vss:	gnd
 *	Vcc:	2v5 - 5v5
 *	WP:		write protect (R/nW)
 *	SCL:	serial clock
 *	SDA:	serial data
 *
 * */

#ifndef MCU_24LC512IP_H
#define MCU_24LC512IP_H

#ifdef STM_HAL	// STM version
// stm libraries
// #include ""
#else  // Arduino version (default)
// arduino libraries
#include "Arduino.h"
#include "Wire.h"
#endif

#define ROM_CAPACITY 0xffff
#define ROM_PAGE_SIZE 128
#define ROM_I2C_BASE_ADDRESS 0x50

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)


typedef enum {
	success = 0,
	addr_nack = 1,
	data_nack = 2,
	transmission_error = 3,
	exceeded_buffer_capacity = 4,
	exceeded_rom_capacity = 5,
	unexpected_size = 6,
	timeout = 7
} _24LC512IP_StatusTypeDef;

typedef struct {
	#ifdef STM_HAL
	I2C_HandleTypeDef* i2c_handle;
	#endif
	uint8_t i2c_addr;
	uint32_t timeout;
} _24LC512IP_TypeDef;


#ifdef STM_HAL
_24LC512IP_TypeDef* new_24LC512IP(I2C_HandleTypeDef* i2c_handle, uint8_t i2c_addr, uint32_t timeout);
#else
_24LC512IP_TypeDef* new_24LC512IP(uint8_t i2c_addr, uint32_t timeout);
#endif

_24LC512IP_StatusTypeDef write(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size);
_24LC512IP_StatusTypeDef read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size);


#endif //MCU_24LC512IP_H