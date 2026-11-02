#include <stddef.h>

#include "protocol.h"
#include "test.h"

#define H PROTOCOL_HEADER
#define F PROTOCOL_FOOTER

/* Matar in alla bytes och returnerar hur många hela paket som togs emot. */
static int feed_all(protocol_parser *parser, const uint8_t *bytes, size_t count)
{
    int packets = 0;
    for (size_t i = 0; i < count; i++) {
        if (protocol_feed(parser, bytes[i])) {
            packets++;
        }
    }
    return packets;
}

#define FEED(parser, ...)                                               \
    feed_all((parser), (const uint8_t[]){__VA_ARGS__},                  \
             sizeof((const uint8_t[]){__VA_ARGS__}))

static void test_giltigt_paket_tas_emot(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 2, 10, 20, F), 1);
    CHECK_EQ(p.length, 2);
    CHECK_EQ(p.data[0], 10);
    CHECK_EQ(p.data[1], 20);
}

static void test_paket_utan_data_tas_emot(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 0, F), 1);
    CHECK_EQ(p.length, 0);
}

static void test_skrap_fore_header_ignoreras(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, 1, 2, 3, H, 1, 42, F), 1);
    CHECK_EQ(p.data[0], 42);
}

static void test_paket_utan_header_kastas(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, 1, 42, F), 0);
}

static void test_paket_med_fel_footer_kastas(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 1, 42, 0x00), 0);
}

static void test_for_langt_paket_kastas(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, PROTOCOL_MAX_DATA + 1, F), 0);
    CHECK_EQ(p.state, PENDING_HEADER);
}

static void test_storsta_tillatna_paket_tas_emot(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, PROTOCOL_MAX_DATA, 1, 2, 3, 4, 5, 6, 7, 8,
                  9, 10, 11, 12, 13, 14, 15, 16, F), 1);
    CHECK_EQ(p.data[PROTOCOL_MAX_DATA - 1], 16);
}

static void test_nasta_paket_tas_emot_efter_fel(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 1, 42, 0x00, H, 1, 43, F), 1);
    CHECK_EQ(p.data[0], 43);
}

static void test_header_och_footer_i_data_ar_vanlig_data(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 2, H, F, F), 1);
    CHECK_EQ(p.data[0], H);
    CHECK_EQ(p.data[1], F);
}

static void test_tva_paket_i_rad_tas_emot(void)
{
    protocol_parser p;
    protocol_init(&p);

    CHECK_EQ(FEED(&p, H, 1, 1, F, H, 1, 2, F), 2);
    CHECK_EQ(p.data[0], 2);
}

int main(void)
{
    RUN(test_giltigt_paket_tas_emot);
    RUN(test_paket_utan_data_tas_emot);
    RUN(test_skrap_fore_header_ignoreras);
    RUN(test_paket_utan_header_kastas);
    RUN(test_paket_med_fel_footer_kastas);
    RUN(test_for_langt_paket_kastas);
    RUN(test_storsta_tillatna_paket_tas_emot);
    RUN(test_nasta_paket_tas_emot_efter_fel);
    RUN(test_header_och_footer_i_data_ar_vanlig_data);
    RUN(test_tva_paket_i_rad_tas_emot);
    return TEST_RESULT();
}
