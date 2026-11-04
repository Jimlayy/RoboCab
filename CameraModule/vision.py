"""Image processing for the camera module, without the camera.

Finds the black tape strips, keeps one stable line per strip and measures the
distance to the nearest line on each side. camera.py feeds it frames from the
Raspberry Pi Camera v2, and the tests in tests/ feed it images.
"""
import cv2
import numpy as np

WIDTH, HEIGHT = 640, 480
DOT_CORDS = (WIDTH//2, 2*HEIGHT//3)

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


def x_at_y(line, y):
    """x where the (infinite) line through (x1, y1)-(x2, y2) crosses height y. Line lenght is controlled in calcDistance"""
    x1, y1, x2, y2 = line
    if y1 == y2:            # horizontal line: crosses every x or none
        return None
    return int(x1 + (y - y1) * (x2 - x1) / (y2 - y1))

def calcdistance(lines):
    """Calculate the distance to the left and right line if any. Return None if a line doesn't exist"""
    min_distance_right = None
    min_distance_left = None
    dot_x, dot_y = DOT_CORDS

    for line in lines:
        x1, y1, x2, y2 = line 
    for line in lines:
        x1, y1, x2, y2 = line
        if not min(y1, y2) - 10 <= dot_y <= max(y1, y2) + 10:   # segment doesn't reach the dot's height
            continue
        x = x_at_y(line, dot_y)
        if x is None:       # horizontal line, e.g. a stop line: says nothing about the sides
            continue

        distance = dot_x - x
        if distance >= 0:
            if min_distance_left is None or distance < min_distance_left:
                min_distance_left = distance
        else:
            distance = -distance
            if min_distance_right is None or distance < min_distance_right:
                min_distance_right = distance

    return min_distance_left, min_distance_right



def process(frame, smoother):
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


    # Draw distances and write it to the screen.
    if distance_left is not None:
        cv2.line(out, DOT_CORDS, (DOT_CORDS[0]-distance_left, DOT_CORDS[1]), (255, 0, 0), 2) # BGR: blue
        x = int(DOT_CORDS[0] - distance_left / 2)
        y = DOT_CORDS[1] + 25
        cv2.putText(out, f"{distance_left:.0f}", (x, y),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.3, (255, 0, 0), 2)

    if distance_right is not None:
        cv2.line(out, DOT_CORDS, (DOT_CORDS[0]+distance_right, DOT_CORDS[1]), (255, 0, 0), 2) # BGR: blue
        x = int(DOT_CORDS[0] + distance_right / 2)
        y = DOT_CORDS[1] + 25
        cv2.putText(out, f"{distance_right:.0f}", (x, y),
            cv2.FONT_HERSHEY_SIMPLEX, 0.3, (255, 0, 0), 2)
    return out
