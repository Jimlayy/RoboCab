import pytest
import numpy as np

import vision

class TestXAtY:
    # Table 1: weak EC testing
    WEAK_EC = [
        pytest.param([200, 100, 200, 400], 250, 200,
                     id="we1_vertical_p1_inside"),
        pytest.param(np.array([300, 400, 100, 100], np.float32), 10, 40,
                     id="we2_tilted_p2_above"),
        pytest.param([100, 300, 500, 300], 450, None,
                     id="we3_horizontal_below"),
    ]

    # Table 2: boundary values around horizontal line (y1 == y2).
    BV_HORIZONTAL = [
        pytest.param([100, 300, 500, 299], 301, -300, id="bv1_y2_below_y1"),
        pytest.param([100, 300, 500, 300], 301, None, id="bv2_y2_equals_y1"),
        pytest.param([100, 300, 500, 301], 301, 500,  id="bv3_y2_above_y1"),
    ]

    # Table 3: boundary values for y around the endpoints of the line [100, 400, 300, 0],
    # where x = 300 - y / 2. int() rounds towards zero.
    BV_Y = [
        pytest.param([100, 400, 300, 0], -1,  300, id="bv4_min_minus_1"),
        pytest.param([100, 400, 300, 0], 0,   300, id="bv5_min"),
        pytest.param([100, 400, 300, 0], 1,   299, id="bv6_min_plus_1"),
        pytest.param([100, 400, 300, 0], 399, 100, id="bv7_max_minus_1"),
        pytest.param([100, 400, 300, 0], 400, 100, id="bv8_max"),
        pytest.param([100, 400, 300, 0], 401, 99,  id="bv9_max_plus_1"),
    ]

    @pytest.mark.parametrize("line, y, expected", WEAK_EC + BV_HORIZONTAL + BV_Y)
    def test_values(self, line, y, expected):
        result = vision.x_at_y(line, y)
        assert result == expected, f"x_at_y({line}, {y}) returned {result}, expected {expected}"

    def test_numpy_input_returns_integer(self):
        # Same input as WE2
        result = vision.x_at_y(np.array([300, 400, 100, 100], np.float32), 10)
        assert type(result) is int, f"type was {type(result).__name__}, expected int"
