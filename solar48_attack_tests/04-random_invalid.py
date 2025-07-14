import random
import string
import serial
import time

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200
with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
	for _ in range(500):
    		arg = ''.join(random.choices(string.printable, k=50))
    		cmd = f'setdate {arg}\r\n'
    		ser.write(cmd.encode('utf-8'))
    		time.sleep(0.01)

