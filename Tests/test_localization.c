#include "localization.h"
#include "test.h"

#define TOLERANCE 100

static void test_ratt_nod_i_intervallet_godkanns(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_STOP_LINE, 0, TOLERANCE),
             LOCALIZATION_NODE_ACCEPTED);
}

static void test_intervallets_kanter_ingar(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_STOP_LINE, TOLERANCE, TOLERANCE),
             LOCALIZATION_NODE_ACCEPTED);
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_STOP_LINE, -TOLERANCE, TOLERANCE),
             LOCALIZATION_NODE_ACCEPTED);
}

static void test_ratt_nod_fore_intervallet_godkanns_inte(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_STOP_LINE, 500, TOLERANCE),
             LOCALIZATION_WAITING);
}

static void test_fel_nodtyp_i_intervallet_ar_vilse(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_INTERSECTION, 0, TOLERANCE),
             LOCALIZATION_LOST);
}

static void test_ingen_nod_fore_intervallets_slut_vantar(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_NONE, 500, TOLERANCE),
             LOCALIZATION_WAITING);
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_NONE, 0, TOLERANCE),
             LOCALIZATION_WAITING);
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_NONE, -TOLERANCE, TOLERANCE),
             LOCALIZATION_WAITING);
}

static void test_ingen_nod_efter_intervallet_ar_vilse(void)
{
    CHECK_EQ(localization_check(NODE_STOP_LINE, NODE_NONE, -TOLERANCE - 1, TOLERANCE),
             LOCALIZATION_LOST);
}

static void test_korsning_godkanns_pa_samma_satt(void)
{
    CHECK_EQ(localization_check(NODE_INTERSECTION, NODE_INTERSECTION, 10, TOLERANCE),
             LOCALIZATION_NODE_ACCEPTED);
}

int main(void)
{
    RUN(test_ratt_nod_i_intervallet_godkanns);
    RUN(test_intervallets_kanter_ingar);
    RUN(test_ratt_nod_fore_intervallet_godkanns_inte);
    RUN(test_fel_nodtyp_i_intervallet_ar_vilse);
    RUN(test_ingen_nod_fore_intervallets_slut_vantar);
    RUN(test_ingen_nod_efter_intervallet_ar_vilse);
    RUN(test_korsning_godkanns_pa_samma_satt);
    return TEST_RESULT();
}
