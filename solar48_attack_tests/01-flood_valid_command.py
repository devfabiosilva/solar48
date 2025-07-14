import serial
import time

SERIAL_PORT = '/dev/ttyACM0'    # Ajuste conforme necessário
BAUD_RATE = 115200

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
    for _ in range(1000):  # Ajuste a quantidade de repetições para o estresse desejado
        ser.write(b'ping\r\n')
        # Opcional: ser.write(b'meminfo\r\n')
        # Opcional: ser.write(b'getdate\r\n')
        time.sleep(0.01)  # Muito curto, simula flood intenso
