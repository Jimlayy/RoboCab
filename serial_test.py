import time
import serial

PORT = "COM6"  # Match your USB-to-UART adapter's COM port
BAUD = 9600    # Matches your 1 MHz double-speed (U2X0) UART setup


def build_packet(speed: int, direction: int, servo_ticks: int, brake: int = 0) -> bytes:
    header = 0xAA
    payload_len = 0x06
    msg_id = 0x01
    servo_hi = (servo_ticks >> 8) & 0xFF
    servo_lo = servo_ticks & 0xFF

    payload = [msg_id, direction, brake, speed, servo_hi, servo_lo]

    checksum = 0
    for b in payload:
        checksum ^= b

    return bytes([header, payload_len, *payload, checksum])


with serial.Serial(PORT, BAUD, timeout=0.5) as ser:
    time.sleep(1.5)  # Allow time if opening the port resets the AVR

    for direction in (0, 1):
        for speed in (0x00, 0x80, 0xF8):
            # Sweeps 125 (1000 us) to 250 (2000 us) in steps of 25 ticks
            # (Change to range(1000, 2001, 250) if your firmware expects microseconds)
            for ticks in range(125, 251, 25):
                pkt = build_packet(speed=speed, direction=direction, servo_ticks=ticks, brake=0)
                ser.write(pkt)
                print(f"dir={direction} speed=0x{speed:02X} brk=0 ticks={ticks} -> {pkt.hex(' ').upper()}")

                time.sleep(0.25)  # Delay between packets (e.g., 0.02 for 50 Hz, 0.5 for slow inspection)