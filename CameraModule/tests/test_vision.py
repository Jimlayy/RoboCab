"""Tester av bildbehandlingen i vision.py, på konstgjorda bilder och linjer.

Bilderna ritas med OpenCV: ljusgrå bakgrund och svart "tejp". Kameran behövs inte.
"""
import cv2
import numpy as np
import pytest

import vision
from vision import LineSmoother, calcdistance, draw_lines, process, strip_lines, tape_mask, x_at_y

BLACK = (20, 20, 20)
TAPE_WIDTH = 15
DOT_X, DOT_Y = vision.DOT_CORDS


def blank(value=200):
    return np.full((vision.HEIGHT, vision.WIDTH, 3), value, np.uint8)


def with_strips(*strips):
    """En bild med en svart remsa per (x1, y1, x2, y2)."""
    frame = blank()
    for x1, y1, x2, y2 in strips:
        cv2.line(frame, (x1, y1), (x2, y2), BLACK, TAPE_WIDTH)
    return frame


def lines_in(frame):
    return strip_lines(tape_mask(frame))


def line(x1, y1, x2, y2):
    return np.array([[x1, y1, x2, y2]], np.float32)


NO_LINES = np.zeros((0, 4), np.float32)


# ---- Tejp och linjer: tape_mask och strip_lines ----

def test_en_svart_remsa_ger_en_linje_langs_remsan():
    lines = lines_in(with_strips((320, 450, 320, 150)))

    assert len(lines) == 1
    x1, y1, x2, y2 = lines[0]
    assert x1 == pytest.approx(320, abs=3)
    assert x2 == pytest.approx(320, abs=3)
    assert abs(y2 - y1) == pytest.approx(300, abs=25)


def test_sned_remsa_ger_linje_med_samma_lutning():
    lines = lines_in(with_strips((150, 450, 350, 150)))

    assert len(lines) == 1
    x1, y1, x2, y2 = lines[0]
    assert (x2 - x1) / (y2 - y1) == pytest.approx(200 / -300, abs=0.05)


def test_tva_remsor_ger_tva_linjer():
    assert len(lines_in(with_strips((150, 450, 150, 150), (490, 450, 490, 150)))) == 2


def test_tom_bild_ger_inga_linjer():
    assert len(lines_in(blank())) == 0


def test_kort_bit_ignoreras():
    assert len(lines_in(with_strips((320, 300, 320, 260)))) == 0


def test_stor_svart_ruta_ignoreras():
    frame = blank()
    cv2.rectangle(frame, (200, 150), (400, 350), BLACK, -1)

    assert len(lines_in(frame)) == 0


def test_stor_mork_yta_ignoreras():
    frame = blank()
    frame[:, :320] = 60  # som en vägg eller skugga över halva bilden

    assert len(lines_in(frame)) == 0


def test_remsa_precis_intill_en_skugga_hittas():
    frame = blank()
    frame[:, 328:] = 90  # skugga som börjar vid remsans kant
    cv2.line(frame, (320, 450), (320, 150), BLACK, TAPE_WIDTH)

    # Utan black-hat växer remsan ihop med skuggan till en för bred fläck.
    assert len(lines_in(frame)) == 1


def test_for_bred_remsa_ignoreras(monkeypatch):
    # Större kärna, så att det bara är MAX_WIDTH som stoppar den breda remsan.
    monkeypatch.setattr(vision, "BH_KERNEL", 121)
    narrow = with_strips((320, 460, 320, 60))
    wide = blank()
    cv2.line(wide, (320, 460), (320, 60), BLACK, vision.MAX_WIDTH + 20)

    assert len(lines_in(narrow)) == 1
    assert len(lines_in(wide)) == 0


def test_morkbla_remsa_ignoreras():
    frame = blank()
    cv2.line(frame, (320, 450), (320, 150), (100, 10, 10), TAPE_WIDTH)  # BGR, mättad färg

    assert len(lines_in(frame)) == 0


def test_remsa_avbruten_av_glans_blir_en_linje():
    frame = with_strips((320, 450, 320, 150))
    cv2.line(frame, (300, 300), (340, 300), (200, 200, 200), 6)  # ljust streck tvärs över

    assert len(lines_in(frame)) == 1


def test_mask_ar_svartvit_och_lika_stor_som_bilden():
    mask = tape_mask(with_strips((320, 450, 320, 150)))

    assert mask.shape == (vision.HEIGHT, vision.WIDTH)
    assert set(np.unique(mask)) <= {0, 255}


# ---- Utjämning mellan bilder: LineSmoother ----

def test_linje_visas_forst_efter_show_after_bilder():
    smoother = LineSmoother()
    shown = [len(smoother.update(line(100, 400, 100, 100))) for _ in range(vision.SHOW_AFTER)]

    assert shown == [0] * (vision.SHOW_AFTER - 1) + [1]


def test_linje_finns_kvar_en_stund_utan_traff():
    smoother = LineSmoother()
    for _ in range(vision.SHOW_AFTER):
        smoother.update(line(100, 400, 100, 100))

    for _ in range(vision.FORGET_AFTER):
        assert len(smoother.update(NO_LINES)) == 1


def test_linje_glomms_efter_forget_after_bilder_utan_traff():
    smoother = LineSmoother()
    for _ in range(vision.SHOW_AFTER):
        smoother.update(line(100, 400, 100, 100))

    for _ in range(vision.FORGET_AFTER + 1):
        shown = smoother.update(NO_LINES)
    assert len(shown) == 0


def test_flyttad_linje_jamnas_ut_med_alpha():
    smoother = LineSmoother()
    for _ in range(vision.SHOW_AFTER):
        smoother.update(line(100, 400, 100, 100))

    shown = smoother.update(line(110, 400, 110, 100))
    expected = (1 - vision.ALPHA) * 100 + vision.ALPHA * 110
    assert shown[0][0] == pytest.approx(expected)
    assert shown[0][2] == pytest.approx(expected)


def test_linje_at_motsatt_hall_slas_inte_ihop():
    smoother = LineSmoother()
    for _ in range(vision.SHOW_AFTER):
        smoother.update(line(100, 400, 100, 100))

    shown = smoother.update(line(100, 100, 100, 400))
    assert shown[0].tolist() == pytest.approx([100, 400, 100, 100])


def test_langt_hopp_blir_en_ny_linje():
    smoother = LineSmoother()
    for _ in range(vision.SHOW_AFTER):
        smoother.update(line(100, 400, 100, 100))

    shown = smoother.update(line(100 + vision.MATCH_DIST + 60, 400, 100 + vision.MATCH_DIST + 60, 100))
    assert len(shown) == 1               # den nya visas inte än
    assert shown[0][0] == pytest.approx(100)  # den gamla finns kvar


def test_tva_linjer_foljs_var_for_sig():
    smoother = LineSmoother()
    both = np.array([[100, 400, 100, 100], [500, 400, 500, 100]], np.float32)
    for _ in range(vision.SHOW_AFTER):
        shown = smoother.update(both)

    assert sorted(shown[:, 0].tolist()) == pytest.approx([100, 500])


# ---- Avstånd till linjerna: x_at_y och calcdistance ----

def test_x_at_y_lodrat():
    assert x_at_y([200, 400, 200, 100], 250) == 200


def test_x_at_y_snett():
    assert x_at_y([100, 480, 300, 0], 320) == 166  # 100 + (320 - 480) * 200 / -480


def test_x_at_y_vagrat_ger_none():
    assert x_at_y([100, 300, 500, 300], 320) is None


def test_avstand_till_linjer_pa_bada_sidor():
    lines = np.array([[DOT_X - 120, 450, DOT_X - 120, 100],
                      [DOT_X + 150, 450, DOT_X + 150, 100]], np.float32)

    assert calcdistance(lines) == (120, 150)


def test_narmaste_linje_pa_varje_sida_valjs():
    lines = np.array([[DOT_X - 220, 450, DOT_X - 220, 100],
                      [DOT_X - 70, 450, DOT_X - 70, 100],
                      [DOT_X + 40, 450, DOT_X + 40, 100],
                      [DOT_X + 200, 450, DOT_X + 200, 100]], np.float32)

    assert calcdistance(lines) == (70, 40)


def test_sned_linje_mats_i_punktens_hojd():
    left, _ = calcdistance(line(100, 480, 300, 0))

    assert left == DOT_X - 166


def test_vagrat_linje_kraschar_inte():
    lines = np.array([[DOT_X - 120, 450, DOT_X - 120, 100],
                      [100, DOT_Y, 500, DOT_Y]], np.float32)  # som en stopplinje

    left, _ = calcdistance(lines)
    assert left == 120


def test_inga_linjer_kraschar_inte():
    left, right = calcdistance(NO_LINES)

    assert left > vision.WIDTH and right > vision.WIDTH  # vad "ingen linje" ska vara är inte bestämt


# ---- Hela kedjan: process och draw_lines ----

def test_process_ger_avstand_efter_show_after_bilder():
    frame = with_strips((DOT_X - 120, 470, DOT_X - 120, 100), (DOT_X + 150, 470, DOT_X + 150, 100))
    smoother = LineSmoother()

    for _ in range(vision.SHOW_AFTER):
        mask, lines, left, right = process(frame, smoother)

    assert len(lines) == 2
    assert left == pytest.approx(120, abs=3)
    assert right == pytest.approx(150, abs=3)


def test_draw_lines_andrar_inte_originalbilden():
    frame = with_strips((320, 450, 320, 150))
    before = frame.copy()

    out = draw_lines(frame, line(320, 450, 320, 150), 120, 150)

    assert out.shape == frame.shape
    assert np.array_equal(frame, before)
    assert not np.array_equal(out, frame)


def test_draw_lines_klarar_att_inga_linjer_hittats():
    left, right = calcdistance(NO_LINES)

    out = draw_lines(blank(), NO_LINES, left, right)
    assert out.shape == (vision.HEIGHT, vision.WIDTH, 3)
