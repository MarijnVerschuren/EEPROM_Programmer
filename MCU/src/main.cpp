#include <Arduino.h>
#include "main.hpp"


_24LC512IP_TypeDef* rom;
uint8_t data[] = {
		1, 0, 2, 4
};  // test data
uint8_t* data_in = new uint8_t[4];

void setup() {
	Serial.begin(115200);
	rom = new_24LC512IP(ROM_I2C_BASE_ADDRESS, 20);
	Serial.println("Start: ");
}

void loop() {
	delay(500);
	auto err = read(rom, 0xff00, data_in, 4);
	if ((uint8_t)err) {
		Serial.print("err: ");
		Serial.println(err);
		return;
	}
	for (uint8_t i = 0; i < 4; i++) {
		Serial.println(data_in[i]);
	}
}