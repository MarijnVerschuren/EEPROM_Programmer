import crc as crc_lib
import serial
import struct
import math
import time
import sys


__all__ = [
	"upload",
	"upload_payload",
	"download",
	"download_payload"
]

crc_config = crc_lib.Configuration(
	width=              32,
	polynomial=         0x04C11DB7,
	init_value=         0xFFFFFFFF,
	final_xor_value=    0x00,
	reverse_input=      False,
	reverse_output=     False
); crc = crc_lib.Calculator(crc_config)


def pack_block(block_type: bytes, address: int, data: bytes = b"") -> bytes:
	packet_data = block_type + struct.pack("<HB", address, 0x00)
	packet_data += struct.pack("<L", crc.checksum(packet_data))
	if block_type == b"W":
		packet_data += data + struct.pack("<L", crc.checksum(data))
	return packet_data


def unpack_block(block: bytes) -> tuple:
	header_type, address, misc, header_crc = struct.unpack("<BHBL", block[:8])
	header_type = bytes([header_type])
	if header_type == b"W":
		return b"W", address, misc, header_crc, block[8:40], block[40:]
	return b"R", address, misc, header_crc
	

def download_payload(address: int, size: int) -> list:
	block_count = math.ceil(size / 32)
	blocks = []
	for i in range(block_count): blocks.append(pack_block(b"R", address + 32 * i))
	return blocks


def download(ser: serial.Serial, address: int, size: int) -> bytes:
	payload = b""
	fail_count = 0
	for block in download_payload(address, size):
		block_data = None
		while not block_data:
			ser.write(block)
			ser.flush()
			while ser.in_waiting < 44 and fail_count < 40:
				time.sleep(0.05); fail_count += 1  # 2 sec max
			if fail_count == 40: fail_count = 0; continue
			block_data = unpack_block(ser.read(44))
			repacked = unpack_block(pack_block(*block_data[:2], block_data[4]))
			block_crc = repacked[3]
			data_crc = repacked[5]
			if block_data[3] == block_crc and block_data[5] == data_crc: break
		payload += block_data[4]
	return payload


def upload_payload(address: int, data: bytes) -> list:
	size = len(data)
	block_count = size // 32
	blocks = []
	for i in range(block_count): blocks.append(pack_block(b"W", address + 32 * i, data[32 * i:32 * (i+1)]))
	if size % 32: blocks.append(pack_block(b"W", address + 32 * block_count, (data[32 * block_count:] + ((32 - (size % 32)) * b"\x00"))))
	return blocks


def upload(ser: serial.Serial, address: int, data: bytes):
	fail_count = 0
	for block in upload_payload(address, data):
		block_data = None
		while not block_data:
			ser.write(block)
			ser.flush()
			while ser.in_waiting < 8 and fail_count < 20:
				time.sleep(0.05); fail_count += 1  # 1 sec
			if fail_count == 20: fail_count = 0; continue
			block_data = unpack_block(ser.read(8))
			block_crc = unpack_block(pack_block(*block_data[:2]))[3]
			if block_data[3] == block_crc: break
