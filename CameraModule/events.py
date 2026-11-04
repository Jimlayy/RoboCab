"""Turns the per-frame line measurements into events and sends them to another
process over a UNIX datagram socket.

An event is only sent when a new state has held for EVENT_FRAMES frames in a row,
so a single noisy frame never triggers a message.

Messages are JSON, one per datagram:
    {"event": "intersection", "side": "left" | "right" | "both"}
    {"event": "stop_line"}
"""
import json
import math
import socket
from collections import deque

EVENT_FRAMES = 3                        # a state must hold this many frames before it counts
STOP_MAX_ANGLE = 20                     # degrees from horizontal for a line to count as a stop line
SOCKET_PATH = "/tmp/vision_events.sock" # the receiving process binds to this path


def is_horizontal(line, max_angle=STOP_MAX_ANGLE):
    """True if the line is within max_angle degrees of horizontal "stop" line."""
    x1, y1, x2, y2 = line
    return abs(y2 - y1) <= abs(x2 - x1) * math.tan(math.radians(max_angle))


def has_stop_line(lines, dot_x):
    """True if a near-horizontal line crosses the road, i.e. passes the dot's x."""
    for line in lines:
        x1, _, x2, _ = line
        if is_horizontal(line) and min(x1, x2) <= dot_x <= max(x1, x2):
            return True
    return False


_sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
_sock.setblocking(False)

_sides = {"history": deque(maxlen=EVENT_FRAMES), "stable": None}   # (left_found, right_found), None = unknown yet
_stop = {"history": deque(maxlen=EVENT_FRAMES), "stable": False}


def debounce(state, value):
    """Feed one frame's value. Returns True on the frame the stable value changes,
    which happens once the new value has been seen EVENT_FRAMES frames in a row."""
    history = state["history"]
    history.append(value)
    full = len(history) == history.maxlen
    if full and value != state["stable"] and all(v == value for v in history):
        state["stable"] = value
        return True
    return False


def send(event, **data):
    try:
        _sock.sendto(json.dumps({"event": event, **data}).encode(), SOCKET_PATH)
    except OSError as e:   # receiver not running, buffer full, etc.
        print(f"Could not send {event}: {e}")


def update(lines, distance_left, distance_right, dot_x):
    """Call once per frame. Sends a message when an intersection or stop line is found."""
    # Intersection: we had lines on both sides, and now one or both are gone
    previous = _sides["stable"]
    state = (distance_left is not None, distance_right is not None)
    if debounce(_sides, state) and previous == (True, True):
        side = {(False, True): "left", (True, False): "right", (False, False): "both"}[state]
        send("intersection", side=side)

    # Stop line: a line across the road has appeared
    if debounce(_stop, has_stop_line(lines, dot_x)) and _stop["stable"]:
        send("stop_line")