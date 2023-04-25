import serial
import time
import os

from lib import *


if __name__ == "__main__":
	ser = serial.Serial(
		'/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0',
		115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout=10,
		xonxoff=False,
	)
	ser.reset_output_buffer()
	ser.reset_input_buffer()
	
	# user io logic
	io_type = None
	address = None
	size = None
	file = None
	while True:
		try:
			if io_type is None:
				print(
					"(0): download",
					"(1): upload",
					"(2): print",
					sep="\n"
				); io_type = int(input(": "))
				if io_type > 2: io_type = None; continue
			if address is None: address = int(input("address: "))
			if io_type == 2:
				if size is None: size = int(input("size: "))
				break
			if io_type:
				path = input("input file path: ")
				if not os.path.exists(path): continue
			else:
				if size is None: size = int(input("size: "))
				path = input("output file path: ")
			file = path
			break
		except KeyboardInterrupt: exit(0)
		except: pass
	
	# ROM io logic
	if io_type < 2:
		with open(file, "rb" if io_type else "wb") as file:
			if io_type: upload(ser, address, file.read(), True)
			else: file.write(download(ser, address, size, True))
	else: print(download(ser, address, size, True))
