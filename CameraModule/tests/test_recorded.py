"""Tester mot inspelade bilder från banan. Se images/README.md för hur en bild läggs till.

Varje bild images/<namn>.png har ett facit i images/<namn>.json. Utan bilder hoppas
testet över, och pytest visar det som "skipped".
"""
import json
from pathlib import Path

import cv2
import pytest

from vision import calcdistance, strip_lines, tape_mask

IMAGES = Path(__file__).parent / "images"
NAMES = sorted(p.stem for p in IMAGES.glob("*.png"))


@pytest.mark.parametrize("name", NAMES)
def test_inspelad_bild(name):
    frame = cv2.imread(str(IMAGES / f"{name}.png"))
    assert frame is not None, f"{name}.png gick inte att läsa"
    expected = json.loads((IMAGES / f"{name}.json").read_text(encoding="utf-8"))
    tolerance = expected.get("tolerance", 10)

    lines = strip_lines(tape_mask(frame))
    assert len(lines) == expected["lines"]

    left, right = calcdistance(lines)
    if "distance_left" in expected:
        assert left == pytest.approx(expected["distance_left"], abs=tolerance)
    if "distance_right" in expected:
        assert right == pytest.approx(expected["distance_right"], abs=tolerance)
