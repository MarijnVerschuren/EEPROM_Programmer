import serial
import time

from lib import *


if __name__ == "__main__":
	ser = serial.Serial('/dev/ttyUSB0', 115200)
	upload(ser, 0, b"daawdawdadksaddsadyavdtyavgdavgdtagvdtawbdawdvawdhvgvtawkdkvyagdyvygkvg")
	download(ser, 0, 32)
	"""
	while True:
		while ser.inWaiting():
			print(ser.read().decode("ascii", errors="ignore"), end="")
		time.sleep(.5)
	"""
	"""
	while True:
		upload(ser, 0, b"daawdawdadksaddsadyavdtyavgdavgdtagvdtawbdawdvawdhvgvtawkdkvyagdyvygkvg")
		print(ser.read().decode("ascii"), end="")
		time.sleep(0.1)
	"""