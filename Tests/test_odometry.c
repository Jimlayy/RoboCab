#include "odometry.h"
#include "test.h"

static void test_tio_pulser_ar_ett_hjulvarv(void)
{
    CHECK_EQ(odometry_wheel_distance_mm(10, 200), 200);
}

static void test_delar_av_varv_raknas(void)
{
    CHECK_EQ(odometry_wheel_distance_mm(5, 200), 100);
    CHECK_EQ(odometry_wheel_distance_mm(3, 200), 60);
}

static void test_inga_pulser_ger_ingen_stracka(void)
{
    CHECK_EQ(odometry_wheel_distance_mm(0, 200), 0);
}

static void test_lang_korning_slar_inte_over(void)
{
    /* N * C ryms inte i 16 bitar, och inte heller i 32 bitar med sista fallet. */
    CHECK_EQ(odometry_wheel_distance_mm(100000, 200), 2000000);
    CHECK_EQ(odometry_wheel_distance_mm(50000000, 200), 1000000000);
}

static void test_total_stracka_ar_medelvarde(void)
{
    CHECK_EQ(odometry_total_distance_mm(100, 200), 150);
    CHECK_EQ(odometry_total_distance_mm(300, 300), 300);
}

static void test_total_stracka_slar_inte_over(void)
{
    CHECK_EQ(odometry_total_distance_mm(4000000000u, 4000000000u), 4000000000u);
}

static void test_hastighet_ar_stracka_genom_tid(void)
{
    CHECK_EQ(odometry_speed_mm_s(100, 500), 200);
    CHECK_EQ(odometry_speed_mm_s(0, 500), 0);
}

static void test_hastighet_med_noll_tid_ar_noll(void)
{
    CHECK_EQ(odometry_speed_mm_s(100, 0), 0);
}

static void test_hjul_med_ny_puls_ar_inte_stilla(void)
{
    CHECK(!odometry_wheel_stopped(1099, 1000, 100));
}

static void test_hjul_utan_puls_ar_stilla(void)
{
    CHECK(odometry_wheel_stopped(1101, 1000, 100));
}

static void test_stillastaende_klarar_att_raknaren_slar_runt(void)
{
    CHECK(!odometry_wheel_stopped(0xFFFFFFF5u, 0xFFFFFFF0u, 100));
    CHECK(!odometry_wheel_stopped(5, 0xFFFFFFF0u, 100));
    CHECK(odometry_wheel_stopped(200, 0xFFFFFFF0u, 100));
}

int main(void)
{
    RUN(test_tio_pulser_ar_ett_hjulvarv);
    RUN(test_delar_av_varv_raknas);
    RUN(test_inga_pulser_ger_ingen_stracka);
    RUN(test_lang_korning_slar_inte_over);
    RUN(test_total_stracka_ar_medelvarde);
    RUN(test_total_stracka_slar_inte_over);
    RUN(test_hastighet_ar_stracka_genom_tid);
    RUN(test_hastighet_med_noll_tid_ar_noll);
    RUN(test_hjul_med_ny_puls_ar_inte_stilla);
    RUN(test_hjul_utan_puls_ar_stilla);
    RUN(test_stillastaende_klarar_att_raknaren_slar_runt);
    return TEST_RESULT();
}
