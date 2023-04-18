import serial
import time
import sys

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
	
	#upload(ser, 0, b"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris non quam id ligula sollicitudin sagittis in in elit. Fusce enim magna, efficitur ac aliquet quis, sollicitudin vitae dui. Phasellus non mi congue, commodo massa non, tempus massa. Suspendisse ullamcorper pharetra tellus at tempus. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Pellentesque condimentum lectus eu laoreet vestibulum. Quisque placerat blandit imperdiet. Maecenas sapien sem, pharetra sed dui non, ultricies varius nisl. Fusce eu nisi sed est pharetra accumsan. Suspendisse nibh justo, luctus sed sodales sed, consectetur id dui. Donec dignissim felis a lectus bibendum iaculis. Sed aliquet cursus diam sit amet hendrerit. Fusce rhoncus eros felis, non volutpat nisl faucibus id. Fusce mollis lacinia pulvinar. Suspendisse scelerisque neque vel facilisis malesuada.")
	print(download(ser, 0, 896))
