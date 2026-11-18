import time
import serial

PORT = "COM7"  # Match your USB-to-UART adapter's COM port
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

    # for direction in (0, 1):
    #     for speed in (0x00, 0x00, 0xF8):
    #         # Sweeps 125 (1000 us) to 250 (2000 us) in steps of 25 ticks
    #         # (Change to range(1000, 2001, 250) if your firmware expects microseconds)
    #         for ticks in range(125, 251, 50):
    #             pkt = build_packet(speed=speed, direction=direction, servo_ticks=ticks, brake=0)
    #             ser.write(pkt)
    #             print(f"dir={direction} speed=0x{speed:02X} brk=0 ticks={ticks} -> {pkt.hex(' ').upper()}")

    #             time.sleep(0.25)  # Delay between packets (e.g., 0.02 for 50 Hz, 0.5 for slow inspection)

    print("FULL LEFT")
    pkt = build_packet(speed=0x00, direction=0x00, servo_ticks=125, brake=0)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR
    print()

    print("FULL RIGHT")
    pkt = build_packet(speed=0x00, direction=0x00, servo_ticks=250, brake=0)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR
    print()

    print("MIDDLE")
    pkt = build_packet(speed=0x00, direction=0x00, servo_ticks=178, brake=0)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR
    print()

    print("FULL SPEED FORWARD")
    pkt = build_packet(speed=0xFF, direction=0x01, servo_ticks=178, brake=0)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR
    print()

    print("FULL SPEED BACKWARDS")
    pkt = build_packet(speed=0xFF, direction=0x00, servo_ticks=178, brake=0)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR
    print()

    print("BRAKE!!!")
    pkt = build_packet(speed=0xFF, direction=0x00, servo_ticks=178, brake=1)
    ser.write(pkt)
    time.sleep(5)  # Allow time if opening the port resets the AVR