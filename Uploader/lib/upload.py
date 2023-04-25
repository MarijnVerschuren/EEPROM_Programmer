import crc as crc_lib
import serial
import struct
import math
import time


__all__ = [
	"upload",
	"upload_payload",
	"download",
	"download_payload"
]


class Block_Data:  # used as unpack function return value
	def __init__(self, block_type, address, misc, block_crc):
		if type(block_type) != bytes: block_type = bytes([block_type])
		self.block_type = block_type
		self.address = address
		self.misc = misc
		self.block_crc = block_crc
		self.data = b""
		self.data_crc = None
	
	def add_data(self, data, data_crc):
		self.data = data
		self.data_crc = data_crc
	
	@property
	def packable(self) -> tuple: return self.block_type, self.address, self.data


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


def unpack_block(block: bytes) -> Block_Data:
	bdat = Block_Data(*struct.unpack("<BHBL", block[:8]))
	if bdat.block_type == b"W": bdat.add_data(block[8:40], block[40:])
	return bdat
	

def download_payload(address: int, size: int) -> list:
	block_count = math.ceil(size / 32)
	blocks = []
	for i in range(block_count): blocks.append(pack_block(b"R", address + 32 * i))
	return blocks


def download(ser: serial.Serial, address: int, size: int, log: bool = False) -> bytes:
	payload = b""; fail_count = 0
	blocks = download_payload(address, size)
	block_count = len(blocks)
	for index, block in enumerate(blocks):
		block_data = None
		while not block_data:
			ser.write(block)
			ser.flush()
			while ser.in_waiting < 44 and fail_count < 40:
				time.sleep(0.05); fail_count += 1  # 2 sec max
			if fail_count == 40: fail_count = 0; continue
			block_data = unpack_block(ser.read(44))
			repacked = unpack_block(pack_block(*block_data.packable))
			if block_data.block_crc == repacked.block_crc and block_data.data_crc == repacked.data_crc: break
		payload += block_data.data
		print(f"download progress: {round(index / block_count * 100)}%", end="\r")
	return payload


def upload_payload(address: int, data: bytes) -> list:
	size = len(data)
	block_count = size // 32
	blocks = []
	for i in range(block_count): blocks.append(pack_block(b"W", address + 32 * i, data[32 * i:32 * (i+1)]))
	if size % 32: blocks.append(pack_block(b"W", address + 32 * block_count, (data[32 * block_count:] + ((32 - (size % 32)) * b"\x00"))))
	return blocks


def upload(ser: serial.Serial, address: int, data: bytes, log: bool = False):
	fail_count = 0
	blocks = upload_payload(address, data)
	block_count = len(blocks)
	for index, block in enumerate(blocks):
		block_data = None
		while not block_data:
			ser.write(block)
			ser.flush()
			while ser.in_waiting < 8 and fail_count < 20:
				time.sleep(0.05); fail_count += 1  # 1 sec max
			if fail_count == 20: fail_count = 0; continue
			block_data = unpack_block(ser.read(8))
			repacked = unpack_block(pack_block(*block_data.packable))
			if block_data.block_crc == repacked.block_crc: break
		print(f"upload progress: {round(index / block_count * 100)}%", end="\r")
