import os
import time
import serial
import sys


def main():
    try:
        read_or_write = sys.argv[1].lower()
        if len(read_or_write) == 1:
            read_or_write = str(read_or_write).lower()
        filename = sys.argv[2]
        port = sys.argv[3]
    except:
        print("Usage: python eeprom.py [R/W] [filename] [port]")
        exit(-1)


    if read_or_write == "w":
        input(f"Flashing {filename}. Press enter to continue")

        file_size = os.path.getsize(filename)
        current_byte = 0
        progress_percent = 0

        # Temporary until page write is implemented
        ser0 = serial.Serial(str(port), 115200, timeout=None)
        time.sleep(1)
        ser0.write(b"w")
        ser0.close()


        with serial.Serial(str(port), 800, timeout=None) as ser:
            with open(filename, "rb") as rom_file:
                time.sleep(1)
                # ser.write(b"w")
                time.sleep(0.01)
                for i in range(file_size):
                    byte_from_file = rom_file.read(1)
                    ser.write(byte_from_file)
                    current_byte += 1
                    progress_percent = (current_byte / file_size) * 100
                    print(f"Progress: {progress_percent:.2f}%", end="\r")
            rom_file.close()
        ser.close()
        sys.exit(0)


    elif read_or_write == "r":
        input(f"Dumping EEPROM to file {filename}. Press enter to continue")

        file_size = 32768
        current_byte = 0
        progress_percent = 0

        with open(filename, "wb") as rom_file:
            with serial.Serial(str(port), 115200, timeout=1) as ser:
                time.sleep(1)
                ser.write(b"r")
                for i in range(file_size):
                    byte_from_ser = ser.read(1)
                    rom_file.write(byte_from_ser)
                    current_byte += 1
                    progress_percent = (current_byte / file_size) * 100
                    print(f"Progress: {progress_percent:.2f}%", end="\r")
            rom_file.close()
        ser.close()
        sys.exit(0)

if __name__ == "__main__":
    main()
