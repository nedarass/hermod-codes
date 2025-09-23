import serial
import time

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

time.sleep(2)

try:
    while True:
        data = "0\r"
        ser.write((data).encode(encoding = "ascii"))  # picoya gonder
        time.sleep(1e-6 * 200)
        
        response = ser.read(len(data))  # cevabi oku
        print("Pico'dan gelen yanıt:", response.decode(encoding = "ascii"))

except KeyboardInterrupt:
    print("\nBağlantı kapatılıyor...")
    ser.close()
