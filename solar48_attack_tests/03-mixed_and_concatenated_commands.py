import serial
import time

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200
with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
	ser.write(b'pingmeminfohelpgetdate\r\n')
	# Ou vários juntos:
	ser.write(b'ping\r\nmeminfo\r\nhelp\r\nsetdate 2025 06 24 12 30\r\n')
