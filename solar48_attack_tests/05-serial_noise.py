import serial
import time
import random

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200
with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
	for _ in range(1000):
		noise = bytes([random.randint(0, 255) for _ in range(20)])
		ser.write(noise)
		time.sleep(0.01)

