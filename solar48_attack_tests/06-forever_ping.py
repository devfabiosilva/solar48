import itertools
import serial
import time

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200
with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
	for i in itertools.count():
    		ser.write(b'ping\r\n')
    		if i % 10 == 0:
        		ser.write(b'meminfo\r\n')
    		time.sleep(0.01)

