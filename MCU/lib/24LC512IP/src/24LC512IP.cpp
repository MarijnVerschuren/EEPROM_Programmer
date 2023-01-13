//
// Created by marijn on 1/12/23.
//
#include "../inc/24LC512IP.h"


#ifdef STM_HAL
_24LC512IP_TypeDef* new_24LC512IP(I2C_HandleTypeDef* i2c_handle, uint8_t i2c_addr, uint32_t timeout){
	_24LC512IP_TypeDef* handle = malloc(sizeof(_24LC512IP_TypeDef));
	handle->i2c_handle = i2c_handle;
	handle->i2c_addr = i2c_addr;
	handle->timeout = timeout;
	return handle;
}

// hidden functions TODO: stm(hal) i2c functions
void i2c_write(_24LC512IP_TypeDef* handle) {
}
void i2c_read(_24LC512IP_TypeDef* handle) {
}
void delay_ms(uint32_t ms) {
}
uint32_t now_ms() { return 0; }

#else
_24LC512IP_TypeDef* new_24LC512IP(uint8_t i2c_addr, uint32_t timeout) {
	_24LC512IP_TypeDef* handle = malloc(sizeof(_24LC512IP_TypeDef));
	handle->i2c_addr = i2c_addr;
	handle->timeout = timeout;
	return handle;
}

// hidden functions
void delay_ms(uint32_t ms) { delay(ms); }
uint32_t now_ms() { return millis(); }

_24LC512IP_StatusTypeDef i2c_write(_24LC512IP_TypeDef* handle, uint16_t rom_addr , uint8_t* buffer, uint16_t size) {
	Wire.beginTransmission(handle->i2c_addr);
	Wire.write(rom_addr >> 8);
	Wire.write(rom_addr & 0xff);
	if (!buffer || !size) { return (_24LC512IP_StatusTypeDef)Wire.endTransmission(); }
	Wire.write(buffer, size); return (_24LC512IP_StatusTypeDef)Wire.endTransmission();
}
_24LC512IP_StatusTypeDef i2c_read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	uint16_t available = Wire.requestFrom(handle->i2c_addr, size);
	if (available != size) { return unexpected_size; }
	uint32_t start = millis();
	for (uint16_t i = 0; i < size; i++) {
		if ((millis() - start) > handle->timeout && \
			!Wire.available()) { return timeout; }
		*buffer++ = Wire.read();
		start = millis();  // update last action time
	}
	return success;
}
_24LC512IP_StatusTypeDef i2c_stat(_24LC512IP_TypeDef* handle) {
	Wire.beginTransmission(handle->i2c_addr);
	return (_24LC512IP_StatusTypeDef)Wire.endTransmission();
}

#endif


_24LC512IP_StatusTypeDef write(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) { return exceeded_buffer_capacity; }
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE - rom_addr % ROM_PAGE_SIZE, size);
		stat = i2c_write(handle, rom_addr, buffer, n);
		if ((uint8_t)stat) { return stat; }

		buffer += n;
		rom_addr += n;
		size -= n;

		delay_ms(5);  // typical page write takes 5ms
		uint32_t start = now_ms();
		while (i2c_stat(handle)) {
			delay_ms(1);  // prevent spamming rom
			if (now_ms() - start > handle->timeout) { return timeout; }
		}
	}
	return success;
}
_24LC512IP_StatusTypeDef read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) { return i2c_write(handle, rom_addr + size, (void*)0, 0); }  // set pointer
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE, size);
		stat = i2c_read(handle, rom_addr, buffer, n);
		if ((uint8_t)stat) { return stat; }

		buffer += n;
		rom_addr += n;
		size -= n;
	}
	return success;
}