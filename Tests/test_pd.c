#include "pd.h"
#include "test.h"

static void test_p_del_ar_kp_ganger_fel(void)
{
    PID_internal_values pid;
    pd_init(&pid, 2, 0);

    CHECK_EQ(pd_update(&pid, 100, 40, 1), 120);
    CHECK_EQ(pid.error, 60);
}

static void test_inget_fel_ger_ingen_styrsignal(void)
{
    PID_internal_values pid;
    pd_init(&pid, 5, 5);

    CHECK_EQ(pd_update(&pid, 50, 50, 1), 0);
}

static void test_fel_at_andra_hallet_ger_negativ_signal(void)
{
    PID_internal_values pid;
    pd_init(&pid, 3, 0);

    CHECK_EQ(pd_update(&pid, 40, 100, 1), -180);
}

static void test_d_del_anvander_forandringen_i_fel(void)
{
    PID_internal_values pid;
    pd_init(&pid, 0, 4);

    CHECK_EQ(pd_update(&pid, 10, 0, 2), 20);  /* 4 / 2 * (10 - 0) */
    CHECK_EQ(pd_update(&pid, 15, 0, 2), 10);  /* 4 / 2 * (15 - 10) */
    CHECK_EQ(pid.previous_error, 10);
}

static void test_liten_kd_avrundas_inte_bort(void)
{
    PID_internal_values pid;
    pd_init(&pid, 0, 1);

    /* Blir 0 om K_D / dT räknas ut först. */
    CHECK_EQ(pd_update(&pid, 100, 0, 4), 25);
}

static void test_p_och_d_summeras(void)
{
    PID_internal_values pid;
    pd_init(&pid, 2, 4);

    CHECK_EQ(pd_update(&pid, 10, 0, 2), 40);  /* 2 * 10 + 4 / 2 * 10 */
}

static void test_delta_t_noll_ger_bara_p_del(void)
{
    PID_internal_values pid;
    pd_init(&pid, 2, 200);

    CHECK_EQ(pd_update(&pid, 10, 0, 0), 20);
}

static void test_stor_positiv_signal_begransas(void)
{
    PID_internal_values pid;
    pd_init(&pid, 255, 0);

    CHECK_EQ(pd_update(&pid, 32767, 0, 1), INT16_MAX);
}

static void test_stor_negativ_signal_begransas(void)
{
    PID_internal_values pid;
    pd_init(&pid, 255, 0);

    CHECK_EQ(pd_update(&pid, -32768, 0, 1), INT16_MIN);
}

static void test_extremt_fel_begransas(void)
{
    PID_internal_values pid;
    pd_init(&pid, 1, 0);

    CHECK_EQ(pd_update(&pid, 32767, -32768, 1), INT16_MAX);
    CHECK_EQ(pid.error, INT16_MAX);
    CHECK_EQ(pd_update(&pid, -32768, 32767, 1), INT16_MIN);
    CHECK_EQ(pid.error, INT16_MIN);
}

static void test_extrem_d_del_begransas(void)
{
    PID_internal_values pid;
    pd_init(&pid, 0, 255);

    pd_update(&pid, -32768, 0, 1);
    CHECK_EQ(pd_update(&pid, 32767, 0, 1), INT16_MAX);
}

int main(void)
{
    RUN(test_p_del_ar_kp_ganger_fel);
    RUN(test_inget_fel_ger_ingen_styrsignal);
    RUN(test_fel_at_andra_hallet_ger_negativ_signal);
    RUN(test_d_del_anvander_forandringen_i_fel);
    RUN(test_liten_kd_avrundas_inte_bort);
    RUN(test_p_och_d_summeras);
    RUN(test_delta_t_noll_ger_bara_p_del);
    RUN(test_stor_positiv_signal_begransas);
    RUN(test_stor_negativ_signal_begransas);
    RUN(test_extremt_fel_begransas);
    RUN(test_extrem_d_del_begransas);
    return TEST_RESULT();
}
