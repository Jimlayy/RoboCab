"""Live video from the Raspberry Pi Camera v2, processed frame by frame.

Press q in the preview window to quit.
"""
import time

import cv2
from picamera2 import Picamera2

WIDTH, HEIGHT = 640, 480

picam2 = Picamera2()
# "RGB888" in picamera2 gives bytes in BGR order, which is what OpenCV expects
config = picam2.create_preview_configuration(main={"size": (WIDTH, HEIGHT), "format": "RGB888"})
picam2.configure(config)
picam2.start()
time.sleep(1)  # let auto exposure and white balance settle

# Optional: lock exposure and white balance so the tape threshold stays stable.
# picam2.set_controls({"AeEnable": False, "AwbEnable": False})


def process(frame):
    """Your calculations go here. Returns a mask to display."""
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (5, 5), 0)
    _, mask = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    return mask


try:
    while True:
        frame = picam2.capture_array()  # numpy array (HEIGHT, WIDTH, 3), BGR
        mask = process(frame)

        cv2.imshow("camera", frame)
        cv2.imshow("tape mask", mask)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
finally:
    picam2.stop()
    cv2.destroyAllWindows()