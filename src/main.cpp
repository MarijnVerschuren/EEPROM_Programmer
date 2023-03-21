#include <Arduino.h>
#include "main.hpp"


_24LC512IP_TypeDef* rom;
uint8_t* data_out = new uint8_t[64]{
		241, 180, 32, 79, 70, 162, 112, 128,
		151, 99, 62, 165, 23, 101, 117, 163,
		161, 140, 7, 185, 97, 186, 72, 66,
		169, 225, 212, 41, 150, 77, 244, 19,
		152, 167, 112, 125, 145, 86, 11, 196,
		61, 104, 238, 118, 223, 163, 40, 146,
		206, 39, 147, 209, 189, 223, 31, 30,
		231, 167, 138, 120, 243, 98, 34, 12
};
uint8_t* data_in = new uint8_t[128]();

void setup() {
	Wire.begin();
	Serial.begin(115200);
	//Wire.setClock(400000);  // 2K pull-up resistors recommended
	rom = new_24LC512IP(ROM_I2C_BASE_ADDRESS, 20);

	Serial.print("Wire response on 0x50: ");
	Serial.println(i2c_stat(rom));
}

void loop() {
	/*uint16_t r_addr = 0xFC0;
	uint16_t w_addr = 0x1000;
	Serial.print("write: ");
	Serial.println(rom_write_buffer(rom, w_addr, data_out, 64, true));
	Serial.print("read: ");
	Serial.println(rom_read_buffer(rom, r_addr, data_in, 128));
	for (uint64_t i = 0; i < 128; i++) {
		Serial.print(data_in[i]);
		if ((i + 1) % 8 == 0) { Serial.print("\n"); }
		else { Serial.print(", "); }
	}
	for(;;) {}*/
}