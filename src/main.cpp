#include <Arduino.h>
#include "main.hpp"


#define TIMEOUT 250


_24LC512IP_TypeDef* rom;
__attribute__((packed)) struct {
	uint8_t type;
	uint16_t addr;
	uint8_t ret;
	uint32_t crc;
} packet_header;
CRC32 crc(0x04C11DB7, 0xFFFFFFFF, 0x00, false, false);
uint8_t buffer[32];
uint32_t buffer_crc;
uint32_t rec_header_crc;
uint32_t rec_buffer_crc;


void setup() {
	Wire.begin();
	Serial.begin(115200);
	Wire.setClock(400000);  // 2K pull-up resistors recommended
	rom = new_24LC512IP(ROM_I2C_BASE_ADDRESS, 20);

	while (i2c_stat(rom)) { delay(10); }
	Serial.print("ROM ready");
}

void loop() {
	while (Serial.available() < sizeof(packet_header));
	Serial.readBytes((uint8_t*)&packet_header, sizeof(packet_header));
	// check xor checksum of address
	crc.restart();
	crc.add((uint8_t*)&packet_header, sizeof(packet_header) - 4);
	rec_header_crc = crc.getCRC();

	if (packet_header.crc != rec_header_crc) { return; }

	uint32_t start;
	if (packet_header.type == 'R') {
		packet_header.type = 'W';
		packet_header.ret = rom_read_buffer(rom, packet_header.addr, buffer, 32);
		crc.restart();
		crc.add((uint8_t*)&packet_header, sizeof(packet_header) - 4);
		packet_header.crc = crc.getCRC();
		crc.restart();
		crc.add(buffer, 32);
		buffer_crc = crc.getCRC();
		Serial.write((uint8_t*)&packet_header, sizeof(packet_header));
		Serial.write(buffer, 32);
		Serial.write((uint8_t*)&buffer_crc, 4);
	} else if (packet_header.type == 'W') {
		start = millis(); while (Serial.available() < 32) { if (millis() - start > TIMEOUT) { return; } }
		Serial.readBytes(buffer, 32);
		start = millis(); while (Serial.available() < 4) { if (millis() - start > TIMEOUT) { return; } }
		Serial.readBytes((uint8_t*)&buffer_crc, 4);
		crc.restart();
		crc.add(buffer, 32);
		rec_buffer_crc = crc.getCRC();
		if (buffer_crc != rec_buffer_crc) { return; }
		packet_header.type = 'R';
		packet_header.ret = rom_write_buffer(rom, packet_header.addr, buffer, 32, true);
		crc.restart();
		crc.add((uint8_t*)&packet_header, sizeof(packet_header) - 4);
		packet_header.crc = crc.getCRC();
		Serial.write((uint8_t*)&packet_header, sizeof(packet_header));  // return code is sent back
	}
}