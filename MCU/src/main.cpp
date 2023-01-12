#include <Arduino.h>
#include "main.hpp"


_24LC512IP_TypeDef* rom;
uint8_t data[] = {
		1, 0, 2, 4
};  // test data

void setup() {
	//Serial.begin(115200);
	rom = new_24LC512IP(ROM_I2C_BASE_ADDRESS, 20);
}

void loop() {
	write(rom, 0xff00, data, 4);
}