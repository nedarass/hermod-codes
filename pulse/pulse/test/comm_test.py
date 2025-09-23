import serial
import struct
import time

# Open serial connection
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)  # Adjust port as needed
time.sleep(2)  # Wait for connection

def send_command(command_id):
    # Create 16-byte command
    command = struct.pack('B', command_id) + b'\x00' * 15
    ser.write(command)
    print(f"Sent command: {command_id}")

# Test commands
print("Opening LED...")
send_command(0x01)  # Open LED
time.sleep(2)

print("Closing LED...")
send_command(0x02)  # Close LED

# Read incoming data
while True:
    line = ser.readline()
    if line:
        print(f"Received: {line.decode().strip()}")
