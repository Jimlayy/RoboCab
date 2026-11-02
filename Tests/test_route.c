#include <stddef.h>

#include "route.h"
#include "test.h"

/* Alla facit är handräknade (designspec tabell 12, krav 21 och 22). */

static void check_path(const route_path *path, const uint8_t *expected, size_t count,
                       uint32_t length, int line)
{
    if (path->node_count != count || path->length != length) {
        printf("  rad %d: fick %d noder med längd %lu, väntade %d noder med längd %lu\n",
               line, path->node_count, (unsigned long)path->length, (int)count,
               (unsigned long)length);
        test_failures++;
        return;
    }
    for (size_t i = 0; i < count; i++) {
        if (path->nodes[i] != expected[i]) {
            printf("  rad %d: nod %d blev %d, väntade %d\n", line, (int)i,
                   path->nodes[i], expected[i]);
            test_failures++;
        }
    }
}

#define CHECK_PATH(path, length, ...)                                   \
    check_path((path), (const uint8_t[]){__VA_ARGS__},                  \
               sizeof((const uint8_t[]){__VA_ARGS__}), (length), __LINE__)

static void test_enkel_vag_hittas(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 20);

    CHECK(route_find(&g, 0, ROUTE_ANY, 2, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 0, 1, 2);
}

static void test_kanter_gar_att_kora_at_bada_hall(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 20);

    CHECK(route_find(&g, 2, ROUTE_ANY, 0, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 2, 1, 0);
}

static void test_kortast_vag_vinner_over_farre_noder(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 3, 100);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 10);
    route_add_edge(&g, 2, 3, 10);

    CHECK(route_find(&g, 0, ROUTE_ANY, 3, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 0, 1, 2, 3);
}

static void test_start_och_mal_samma_nod(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 2);
    route_add_edge(&g, 0, 1, 10);

    CHECK(route_find(&g, 0, ROUTE_ANY, 0, ROUTE_ANY, &p));
    CHECK_PATH(&p, 0, 0);
}

static void test_ingen_vag_ger_false(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);

    CHECK(!route_find(&g, 0, ROUTE_ANY, 2, ROUTE_ANY, &p));
}

static void test_ogiltig_nod_avvisas(void)
{
    route_graph g;
    route_init(&g, 2);

    CHECK(!route_add_edge(&g, 0, 2, 10));
}

static void test_lika_lang_vag_valjer_farre_svangar(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 3, 10);
    route_add_edge(&g, 0, 2, 10);
    route_add_edge(&g, 2, 3, 10);

    route_set_straight(&g, 0, 2, 3);
    CHECK(route_find(&g, 0, ROUTE_ANY, 3, ROUTE_ANY, &p));
    CHECK_PATH(&p, 20, 0, 2, 3);
}

static void test_lika_lang_vag_valjer_farre_svangar_andra_hallet(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 3, 10);
    route_add_edge(&g, 0, 2, 10);
    route_add_edge(&g, 2, 3, 10);

    route_set_straight(&g, 3, 1, 0);
    CHECK(route_find(&g, 0, ROUTE_ANY, 3, ROUTE_ANY, &p));
    CHECK_PATH(&p, 20, 0, 1, 3);
}

static void test_langd_gar_fore_svangar(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 3, 10);
    route_add_edge(&g, 0, 2, 10);
    route_add_edge(&g, 2, 3, 11);

    route_set_straight(&g, 0, 2, 3);
    CHECK(route_find(&g, 0, ROUTE_ANY, 3, ROUTE_ANY, &p));
    CHECK_PATH(&p, 20, 0, 1, 3);
}

/* Vägarna möts i 3 och fortsätter till målet 4, så valet avgörs inte vid målet. */
static void add_paths_that_merge_before_goal(route_graph *g, uint16_t last_length)
{
    route_init(g, 5);
    route_add_edge(g, 0, 1, 10);
    route_add_edge(g, 1, 3, 10);
    route_add_edge(g, 0, 2, 10);
    route_add_edge(g, 2, 3, last_length);
    route_add_edge(g, 3, 4, 10);
}

static void test_farre_svangar_galler_aven_fore_malet(void)
{
    route_graph g;
    route_path p;
    add_paths_that_merge_before_goal(&g, 10);

    route_set_straight(&g, 0, 2, 3);
    CHECK(route_find(&g, 0, ROUTE_ANY, 4, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 0, 2, 3, 4);
}

static void test_farre_svangar_galler_aven_fore_malet_andra_hallet(void)
{
    route_graph g;
    route_path p;
    add_paths_that_merge_before_goal(&g, 10);

    route_set_straight(&g, 0, 1, 3);
    CHECK(route_find(&g, 0, ROUTE_ANY, 4, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 0, 1, 3, 4);
}

static void test_langd_gar_fore_svangar_aven_fore_malet(void)
{
    route_graph g;
    route_path p;
    add_paths_that_merge_before_goal(&g, 11);

    route_set_straight(&g, 0, 2, 3);
    CHECK(route_find(&g, 0, ROUTE_ANY, 4, ROUTE_ANY, &p));
    CHECK_PATH(&p, 30, 0, 1, 3, 4);
}

static void test_bilen_vander_inte_vid_start(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 10);
    route_add_edge(&g, 0, 3, 50);
    route_add_edge(&g, 3, 2, 50);

    CHECK(route_find(&g, 0, 1, 2, ROUTE_ANY, &p));
    CHECK_PATH(&p, 100, 0, 3, 2);
}

static void test_bilen_vander_inte_pa_vagen(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 10);

    /* Enda vägen tillbaka till 0 kräver en vändning i 2. */
    CHECK(!route_find(&g, 1, 0, 0, ROUTE_ANY, &p));
}

static void test_ankomst_fran_ratt_hall(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 3, 10);
    route_add_edge(&g, 0, 2, 100);
    route_add_edge(&g, 2, 3, 100);

    CHECK(route_find(&g, 0, ROUTE_ANY, 3, 2, &p));
    CHECK_PATH(&p, 200, 0, 2, 3);
}

static void test_ankomst_fran_ratt_hall_kan_krava_slinga(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 4);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 10);
    route_add_edge(&g, 2, 3, 10);
    route_add_edge(&g, 3, 1, 10);

    /* Målet 1 måste passeras en gång för att kunna ankomma från 3. */
    CHECK(route_find(&g, 0, ROUTE_ANY, 1, 3, &p));
    CHECK_PATH(&p, 40, 0, 1, 2, 3, 1);
}

static void test_omojlig_ankomstriktning_ger_false(void)
{
    route_graph g;
    route_path p;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 10);

    /* Att ankomma från 2 kräver en vändning i 2. */
    CHECK(!route_find(&g, 0, ROUTE_ANY, 1, 2, &p));
}

static void test_helt_uppdrag_upphamtning_och_avlamning(void)
{
    route_graph g;
    route_path pickup;
    route_path dropoff;
    route_init(&g, 3);
    route_add_edge(&g, 0, 1, 10);
    route_add_edge(&g, 1, 2, 100);
    route_add_edge(&g, 0, 2, 5);

    CHECK(route_find(&g, 0, ROUTE_ANY, 1, 0, &pickup));
    CHECK_PATH(&pickup, 10, 0, 1);

    /* 1-0-2 är kortare men kräver att bilen vänder efter upphämtningen. */
    CHECK(route_find(&g, 1, 0, 2, ROUTE_ANY, &dropoff));
    CHECK_PATH(&dropoff, 100, 1, 2);
}

int main(void)
{
    RUN(test_enkel_vag_hittas);
    RUN(test_kanter_gar_att_kora_at_bada_hall);
    RUN(test_kortast_vag_vinner_over_farre_noder);
    RUN(test_start_och_mal_samma_nod);
    RUN(test_ingen_vag_ger_false);
    RUN(test_ogiltig_nod_avvisas);
    RUN(test_lika_lang_vag_valjer_farre_svangar);
    RUN(test_lika_lang_vag_valjer_farre_svangar_andra_hallet);
    RUN(test_langd_gar_fore_svangar);
    RUN(test_farre_svangar_galler_aven_fore_malet);
    RUN(test_farre_svangar_galler_aven_fore_malet_andra_hallet);
    RUN(test_langd_gar_fore_svangar_aven_fore_malet);
    RUN(test_bilen_vander_inte_vid_start);
    RUN(test_bilen_vander_inte_pa_vagen);
    RUN(test_ankomst_fran_ratt_hall);
    RUN(test_ankomst_fran_ratt_hall_kan_krava_slinga);
    RUN(test_omojlig_ankomstriktning_ger_false);
    RUN(test_helt_uppdrag_upphamtning_och_avlamning);
    return TEST_RESULT();
}
