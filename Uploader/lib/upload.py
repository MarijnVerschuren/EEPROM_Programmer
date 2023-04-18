import crc as crc_lib
import serial
import struct
import time


__all__ = [
	"upload",
	"download"
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
	return struct.unpack("<HBL", block)

	
def read_block(ser: serial.Serial, addr: int) -> bytes:  # TODO: finish func (currently in debug state)
	ser.write(pack_block(b"R", addr))
	ser.read_until(b"W")
	return ser.read_all()


def download(ser: serial.Serial, address: int, size: int) -> bytes:  # TODO: implement
	print(pack_block(b"R", address).hex())


def upload(ser: serial.Serial, address: int, data: bytes):
	size = len(data)
	block_count = size // 32
	blocks = []
	for i in range(block_count): blocks.append(pack_block(b"W", address + 32 * i, data[32 * i:32 * (i+1)]))
	if size % 32: blocks.append(pack_block(b"W", address + 32 * block_count, (data[32 * block_count:] + ((32 - (size % 32)) * b"\x00"))))
	for block in blocks:
		print(block.hex())
		"""while True:
			time.sleep(0.1)
			ser.write(block)
			print(ser.read_all())
			if ser.in_waiting >= 8:
				print(",")
				block = ser.read(8)
				data = unpack_block(block)
				print(data)
				if data[2] != crc.checksum(block[:4]):
					continue
				print("done", block, data)
		"""