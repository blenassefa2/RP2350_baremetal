#!/usr/bin/env python3
import serial
import sys

PORT =  "/dev/cu.usbmodem11102"  # change to your device
BAUD = 115200                     # match your device's baud rate

def parse_input(s):
    """
    Accepts either:
      - hex bytes: "AB CD 01" or "ABCD01"
      - a plain string to send as-is (prefix with 'text:')
    """
    s = s.strip()
    if s.lower().startswith("text:"):
        return s[5:].encode()
    # strip spaces, treat as hex
    hex_str = s.replace(" ", "")
    try:
        return bytes.fromhex(hex_str)
    except ValueError:
        print("Invalid hex input. Use hex like 'AB CD 01' or prefix with 'text:' for raw text.", file=sys.stderr)
        sys.exit(1)

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
    except serial.SerialException as e:
        print(f"Could not open {PORT}: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Connected to {PORT} @ {BAUD} baud.")
    print("Enter bytes as hex (e.g. 'AB CD 01') or 'text:hello' to send a string.")
    print("Ctrl+C to quit.\n")

    try:
        while True:
            raw = input("> ")
            if not raw:
                continue
            data = parse_input(raw)
            ser.write(data)
            print(f"Sent {len(data)} byte(s): {data.hex(' ')}")
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()