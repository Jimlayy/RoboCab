"""Live video from the Raspberry Pi Camera v2, processed frame by frame.

Finds the black tape strips and draws one stable line per strip.
Press q in the preview window to quit.
"""
import time

import cv2
import numpy as np
from picamera2 import Picamera2

WIDTH, HEIGHT = 640, 480
DOT_CORDS = (WIDTH//2, 2*HEIGHT//3)
TUNE = False  # True: show the mask + sliders to tune the values live

# ---- What counts as tape ----
V_MAX = 110       # brightness (HSV V, 0-255) must be below this -> "black"
S_MAX = 100       # saturation must be below this -> grey/black, not wooden frame or wall
BH_MIN = 40       # pixel must be at least this much darker than its surroundings
BH_KERNEL = 41    # px, must be wider than the tape appears in the image

# ---- What counts as a strip ----
MIN_LENGTH = 60   # px, shorter blobs are discarded
MIN_ASPECT = 3.0  # length / width, a strip must be elongated
MAX_WIDTH = 40    # px, wider blobs are not tape

# ---- Smoothing between frames ----
ALPHA = 0.4       # 0..1, lower = calmer but slower lines
MATCH_DIST = 40   # px, max movement of a line's midpoint between frames
SHOW_AFTER = 3    # a line must be seen in this many frames before it's shown
FORGET_AFTER = 5  # a line is dropped after this many frames without a match

picam2 = Picamera2()
# "RGB888" in picamera2 gives bytes in BGR order, which is what OpenCV expects
config = picam2.create_preview_configuration(main={"size": (WIDTH, HEIGHT), "format": "RGB888"})
picam2.configure(config)
picam2.start()
time.sleep(1)  # let auto exposure and white balance settle

# Lock exposure and white balance so the thresholds stay stable.
picam2.set_controls({"AeEnable": False, "AwbEnable": False})

OPEN_K = np.ones((3, 3), np.uint8)
CLOSE_K = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 41))  # joins strips split by glare


def tape_mask(frame):
    """White where the image is black tape."""
    hsv = cv2.cvtColor(cv2.GaussianBlur(frame, (5, 5), 0), cv2.COLOR_BGR2HSV)
    s, v = hsv[..., 1], hsv[..., 2]
    # Black-hat = (closing - image): large where something is darker than its surroundings.
    # Large dark areas (wall, shadows) get low values, narrow dark strips get high values.
    bh_k = cv2.getStructuringElement(cv2.MORPH_RECT, (BH_KERNEL, BH_KERNEL))
    blackhat = cv2.morphologyEx(v, cv2.MORPH_BLACKHAT, bh_k, borderType=cv2.BORDER_REPLICATE)
    mask = ((v < V_MAX) & (s < S_MAX) & (blackhat > BH_MIN)).astype(np.uint8) * 255
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, OPEN_K)    # remove specks
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, CLOSE_K)  # fill holes and gaps
    return mask


def strip_lines(mask):
    """One centre line per elongated blob. Returns an (N, 4) array of x1, y1, x2, y2."""
    lines = []
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    for c in contours:
        _, (w, h), _ = cv2.minAreaRect(c)
        length, width = max(w, h), min(w, h)
        if length < MIN_LENGTH or width > MAX_WIDTH or length < MIN_ASPECT * max(width, 1):
            continue
        vx, vy, x0, y0 = cv2.fitLine(c, cv2.DIST_L2, 0, 0.01, 0.01).ravel()
        t = (c[:, 0, 0] - x0) * vx + (c[:, 0, 1] - y0) * vy  # project points onto the line
        lines.append([x0 + vx * t.min(), y0 + vy * t.min(), x0 + vx * t.max(), y0 + vy * t.max()])
    return np.array(lines, dtype=np.float32).reshape(-1, 4)


class LineSmoother:
    """Matches lines to the previous frame's and low-pass filters their endpoints."""

    def __init__(self):
        self.tracks = []  # each: {"line": array(4), "hits": int, "misses": int}

    def update(self, lines):
        used = set()
        for tr in self.tracks:
            mid = (tr["line"][:2] + tr["line"][2:]) / 2
            best, best_d = None, MATCH_DIST
            for i, l in enumerate(lines):
                d = np.linalg.norm((l[:2] + l[2:]) / 2 - mid)
                if i not in used and d < best_d:
                    best, best_d = i, d
            if best is None:
                tr["misses"] += 1
                continue
            used.add(best)
            l = lines[best]
            if np.dot(l[2:] - l[:2], tr["line"][2:] - tr["line"][:2]) < 0:
                l = l[[2, 3, 0, 1]]  # same direction as the track before averaging
            tr["line"] = (1 - ALPHA) * tr["line"] + ALPHA * l
            tr["hits"] += 1
            tr["misses"] = 0
        self.tracks = [t for t in self.tracks if t["misses"] <= FORGET_AFTER]
        for i, l in enumerate(lines):
            if i not in used:
                self.tracks.append({"line": l.copy(), "hits": 1, "misses": 0})
        shown = [t["line"] for t in self.tracks if t["hits"] >= SHOW_AFTER]
        return np.array(shown, dtype=np.float32).reshape(-1, 4)


smoother = LineSmoother()

def x_at_y(line, y):
    """x where the (infinite) line through (x1, y1)-(x2, y2) crosses height y."""
    x1, y1, x2, y2 = line
    if y1 == y2:            # horizontal line: crosses every x or none
        return None
    return int(x1 + (y - y1) * (x2 - x1) / (y2 - y1))

def calcdistance(lines):
    """Calculate the distance to the left and right line if any"""
    min_distance_right = 100000
    min_distance_left = 10000

    for line in lines:
        x = x_at_y(line, DOT_CORDS[1])
        distance = DOT_CORDS[0] - x 
        if distance >= 0:
            if distance <= min_distance_left:
                min_distance_left = distance
        else:
            distance = distance*-1
            if distance <= min_distance_right:
                min_distance_right = distance
    return min_distance_left, min_distance_right



def process(frame):
    """Your calculations go here. Returns the mask and the smoothed lines."""
    mask = tape_mask(frame)
    lines = smoother.update(strip_lines(mask))
    distance_left, distance_right = calcdistance(lines)
    return mask, lines, distance_left, distance_right


def draw_lines(frame, lines, distance_left, distance_right):
    """Draw detected road lines in red on a copy of the frame."""
    out = frame.copy()
    for x1, y1, x2, y2 in lines.reshape(-1, 4).astype(int):
        cv2.line(out, (x1, y1), (x2, y2), (0, 0, 255), 3)  # BGR: red

    # Draw midpint    
    cv2.circle(out, DOT_CORDS, 5, (0, 255, 0), cv2.FILLED)
    try:
    # Draw distances
        cv2.line(out, DOT_CORDS, (DOT_CORDS[0]-distance_left, DOT_CORDS[1]), (255, 0, 0), 2) # BGR: blue
        cv2.putText(out, str(distance_left), (DOT_CORDS[0] - distance_left // 2, DOT_CORDS[1] + 10))

        cv2.line(out, DOT_CORDS, (DOT_CORDS[0]+distance_right, DOT_CORDS[1]), (255, 0, 0), 2) # BGR: blue

    except:
        print("Lines not found")
    return out


if TUNE:
    cv2.namedWindow("tape mask")
    for name, val, top in [("V_MAX", V_MAX, 255), ("S_MAX", S_MAX, 255), ("BH_MIN", BH_MIN, 255)]:
        cv2.createTrackbar(name, "tape mask", val, top, lambda _: None)

try:
    while True:
        frame = picam2.capture_array()  # numpy array (HEIGHT, WIDTH, 3), BGR
        if TUNE:
            V_MAX = cv2.getTrackbarPos("V_MAX", "tape mask")
            S_MAX = cv2.getTrackbarPos("S_MAX", "tape mask")
            BH_MIN = cv2.getTrackbarPos("BH_MIN", "tape mask")
        mask, lines, distance_left, distance_right = process(frame)


        cv2.imshow("camera", draw_lines(frame, lines, distance_left, distance_right))
        if TUNE:
            cv2.imshow("tape mask", mask)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
finally:
    picam2.stop()
    cv2.destroyAllWindows()