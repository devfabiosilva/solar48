import serial
import time

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
    for _ in range(500):
        ser.write(b'invalido\r\n')
        ser.write(b'help\r\n')
        ser.write(b'setdate abc xyz\r\n')
        ser.write(b'meminfo\r\n')
        time.sleep(0.01)
