"""Live video from the Raspberry Pi Camera v2, processed frame by frame.

Finds the black tape strips and draws one stable line per strip.
Press q in the preview window to quit.

The image processing is in vision.py, so that it can be tested without a camera.
"""
import time

import cv2
from picamera2 import Picamera2
import events
import vision
from vision import DOT_CORDS, HEIGHT, WIDTH, LineSmoother, draw_lines, process

TUNE = False  # True: show the mask + sliders to tune the values live
PROCESS_EVERY = 2   # process every 2nd frame


picam2 = Picamera2()
# "RGB888" in picamera2 gives bytes in BGR order, which is what OpenCV expects
config = picam2.create_preview_configuration(main={"size": (WIDTH, HEIGHT), "format": "RGB888"})
picam2.configure(config)
picam2.start()
time.sleep(1)  # let auto exposure and white balance settle

# Lock exposure and white balance so the thresholds stay stable.
picam2.set_controls({"AeEnable": False, "AwbEnable": False})

smoother = LineSmoother()

if TUNE:
    cv2.namedWindow("tape mask")
    for name, val, top in [("V_MAX", vision.V_MAX, 255), ("S_MAX", vision.S_MAX, 255), ("BH_MIN", vision.BH_MIN, 255)]:
        cv2.createTrackbar(name, "tape mask", val, top, lambda _: None)



try:
    frame_no = 0
    while True:
        frame = picam2.capture_array()  # numpy array (HEIGHT, WIDTH, 3), BGR
        frame_no += 1
        if frame_no % PROCESS_EVERY:
            continue
        if TUNE:
            # vision.py reads these at every call, so the sliders take effect directly.
            vision.V_MAX = cv2.getTrackbarPos("V_MAX", "tape mask")
            vision.S_MAX = cv2.getTrackbarPos("S_MAX", "tape mask")
            vision.BH_MIN = cv2.getTrackbarPos("BH_MIN", "tape mask")
        mask, lines, distance_left, distance_right = process(frame, smoother)
        events.update(lines, distance_left, distance_right, DOT_CORDS)


        cv2.imshow("camera", draw_lines(frame, lines, distance_left, distance_right))
        if TUNE:
            cv2.imshow("tape mask", mask)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
finally:
    picam2.stop()
    cv2.destroyAllWindows()
