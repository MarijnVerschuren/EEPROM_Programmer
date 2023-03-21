//
// Created by marijn on 1/12/23.
//
#include "../inc/24LC512IP.h"


_24LC512IP_TypeDef* new_24LC512IP(uint8_t i2c_addr, uint32_t timeout) {
	_24LC512IP_TypeDef* handle = (_24LC512IP_TypeDef*)malloc(sizeof(_24LC512IP_TypeDef));
	handle->i2c_addr = i2c_addr;
	handle->timeout = timeout;
	return handle;
}
_24LC512IP_StatusTypeDef rom_write(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t byte) {
	Wire.beginTransmission(handle->i2c_addr);
	Wire.write(rom_addr >> 8);
	Wire.write(rom_addr & 0xff);
	Wire.write(byte);
	return (_24LC512IP_StatusTypeDef)Wire.endTransmission();
}
_24LC512IP_StatusTypeDef rom_read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* byte) {
	Wire.beginTransmission(handle->i2c_addr);
	Wire.write(rom_addr >> 8);
	Wire.write(rom_addr & 0xff);
	_24LC512IP_StatusTypeDef stat = (_24LC512IP_StatusTypeDef)Wire.endTransmission();
	Wire.requestFrom(handle->i2c_addr, 1u);
	*byte = Wire.read();
	return stat;
}
_24LC512IP_StatusTypeDef i2c_stat(_24LC512IP_TypeDef* handle) {
	Wire.beginTransmission(handle->i2c_addr);
	return (_24LC512IP_StatusTypeDef)Wire.endTransmission();
}

_24LC512IP_StatusTypeDef rom_write_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size, bool check) {
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) { return exceeded_buffer_capacity; }
	const uint16_t start_addr = rom_addr;
	const uint16_t start_size = size;
	uint8_t* start_buffer = buffer;
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE - (rom_addr % ROM_PAGE_SIZE), size);
		Wire.beginTransmission(handle->i2c_addr);
		Wire.write(rom_addr >> 8);
		Wire.write(rom_addr & 0xff);
		Wire.write(buffer, n);
		stat = (_24LC512IP_StatusTypeDef)Wire.endTransmission();
		if ((uint8_t)stat) { return stat; }

		buffer += n;
		rom_addr += n;
		size -= n;

		delay(5);  // typical page write takes 5ms
		uint32_t start = millis();
		while (i2c_stat(handle)) {
			delay(1);  // prevent spamming rom
			if (millis() - start > handle->timeout) { return timeout; }
		}
	}
	if (check) {
		delay(5);  // wait an other 5 ms just to be sure
		return rom_write_buffer_check(handle, start_addr, start_buffer, start_size);
	}
	return success;
}
_24LC512IP_StatusTypeDef rom_read_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) {  // set pointer
		Wire.beginTransmission(handle->i2c_addr);
		Wire.write(rom_addr >> 8);
		Wire.write(rom_addr & 0xff);
		return (_24LC512IP_StatusTypeDef)Wire.endTransmission();
	}
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE - (rom_addr % ROM_PAGE_SIZE), size);
		Wire.beginTransmission(handle->i2c_addr);
		Wire.write(rom_addr >> 8);
		Wire.write(rom_addr & 0xff);
		stat = (_24LC512IP_StatusTypeDef)Wire.endTransmission();
		Wire.requestFrom(handle->i2c_addr, n);
		if ((uint8_t)stat) { return stat; }
		for (uint8_t i = 0; i < n; i++) { buffer[i] = Wire.read(); }

		buffer += n;
		rom_addr += n;
		size -= n;
	}
	return success;
}

_24LC512IP_StatusTypeDef rom_write_buffer_check(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	uint8_t* check_buffer = new uint8_t[size];
	_24LC512IP_StatusTypeDef stat;
	stat = rom_read_buffer(handle, rom_addr, check_buffer, size);
	if (stat) { return stat; }
	for (uint16_t i = 0; i < size; i++) {
		if (buffer[i] != check_buffer[i]) {
			stat = rom_write(handle, rom_addr + i, buffer[i]);
			if (stat) { return stat; }
			delay(5);  // write takes 5ms typically
		}
	}
	return success;
}