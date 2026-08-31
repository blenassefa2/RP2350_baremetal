#!/usr/bin/env python3
import serial
import sys

PORT = "/dev/cu.usbmodem11102"   # change to your device, e.g. /dev/ttyACM0 or COM3
# PORT = "/dev/cu.usbmodem1102"   # change to your device, e.g. /dev/ttyACM0 or COM3
BAUD = 115200              # change to match your device

def main():
    try:
        ser = serial.Serial()
        ser.port = PORT
        ser.baudrate = BAUD
        ser.timeout = 1
        ser.dtr = True   # deassert before opening — avoid toggling target reset
        ser.rts = True
        ser.open()
 
    except serial.SerialException as e:
        print(f"Could not open {PORT}: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Listening on {PORT} @ {BAUD} baud. Ctrl+C to stop.\n")

    try:
        while True:
            data = ser.read(ser.in_waiting or 1)  # blocks up to timeout, else reads what's available
            if data:
                hex_str = data.hex(" ")
                bin_str = " ".join(f"{b:08b}" for b in data)
                print(f"HEX: {hex_str}")
                # print(f"BIN: {bin_str}")
                print("-" * 40)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()