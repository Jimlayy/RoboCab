#include "decision.h"
#include "test.h"

static void test_fri_vag_i_mitten_kor_rakt(void)
{
    decision_input in = {0};
    decision_output out = decision_decide(&in);

    CHECK_EQ(out.directive, DIRECTIVE_DRIVE);
    CHECK_EQ(out.steering, 0);
}

static void test_bil_till_hoger_svanger_vanster(void)
{
    decision_input in = {.road_offset_mm = 30};
    decision_output out = decision_decide(&in);

    CHECK_EQ(out.directive, DIRECTIVE_DRIVE);
    CHECK(out.steering < 0);
}

static void test_bil_till_vanster_svanger_hoger(void)
{
    decision_input in = {.road_offset_mm = -30};
    decision_output out = decision_decide(&in);

    CHECK_EQ(out.directive, DIRECTIVE_DRIVE);
    CHECK(out.steering > 0);
}

static void test_hinder_stoppar_bilen(void)
{
    decision_input in = {.obstacle = true};
    decision_output out = decision_decide(&in);

    CHECK_EQ(out.directive, DIRECTIVE_STOP);
    CHECK_EQ(out.steering, 0);
}

static void test_bilen_kor_igen_nar_hindret_ar_borta(void)
{
    decision_input in = {.obstacle = true};
    CHECK_EQ(decision_decide(&in).directive, DIRECTIVE_STOP);

    in.obstacle = false;
    CHECK_EQ(decision_decide(&in).directive, DIRECTIVE_DRIVE);
}

static void test_stopplinje_stoppar_bilen(void)
{
    decision_input in = {.stop_line = true};

    CHECK_EQ(decision_decide(&in).directive, DIRECTIVE_STOP);
}

static void test_vilse_stoppar_bilen(void)
{
    decision_input in = {.lost = true};

    CHECK_EQ(decision_decide(&in).directive, DIRECTIVE_STOP);
}

static void test_hinder_stoppar_aven_nar_bilen_ligger_snett(void)
{
    decision_input in = {.road_offset_mm = 200, .obstacle = true};
    decision_output out = decision_decide(&in);

    CHECK_EQ(out.directive, DIRECTIVE_STOP);
    CHECK_EQ(out.steering, 0);
}

int main(void)
{
    RUN(test_fri_vag_i_mitten_kor_rakt);
    RUN(test_bil_till_hoger_svanger_vanster);
    RUN(test_bil_till_vanster_svanger_hoger);
    RUN(test_hinder_stoppar_bilen);
    RUN(test_bilen_kor_igen_nar_hindret_ar_borta);
    RUN(test_stopplinje_stoppar_bilen);
    RUN(test_vilse_stoppar_bilen);
    RUN(test_hinder_stoppar_aven_nar_bilen_ligger_snett);
    return TEST_RESULT();
}
