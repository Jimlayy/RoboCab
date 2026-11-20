#include "unity.h"
#include "parser.h"
#include <string.h>
#include <stddef.h>

// Result variables
static int packet_count;
static uint8_t packet_len;
static uint8_t packet_data[UART_MAX_PAYLOAD_LENGTH];

/* Results for test_two_parsers_are_independent, one set per parser. */
static int count_a, count_b;
static uint8_t len_a, len_b;
static uint8_t data_a[UART_MAX_PAYLOAD_LENGTH];
static uint8_t data_b[UART_MAX_PAYLOAD_LENGTH];


// Store the result values
static void on_packet(const uint8_t *data, uint8_t len)
{
    packet_count++;
    packet_len = len;
    for (uint8_t i = 0; i < len; i++) {
        packet_data[i] = data[i];
    }
}

/* For test_two_parsers_are_independent */
static void on_packet_a(const uint8_t *data, uint8_t len)
{
    count_a++;
    len_a = len;
    memcpy(data_a, data, len);
}

/* For test_two_parsers_are_independent */
static void on_packet_b(const uint8_t *data, uint8_t len)
{
    count_b++;
    len_b = len;
    memcpy(data_b, data, len);
}

static parser_t parser;

// Helper function, feed a sequence of bytes to the parser and return the result of the last byte
static int16_t feed(const uint8_t *bytes, size_t count)
{
    int16_t last = 0;
    for (size_t i = 0; i < count; i++) {
        last = parser_feed(&parser, bytes[i]);
    }
    return last;   /* return value of last byte */
}

void setUp(void)        /* is run before every test, reset everything */
{
    packet_count = 0;
    packet_len = 0;
    memset(packet_data, 0, sizeof packet_data);
    parser_init(&parser, on_packet);
}

void tearDown(void)     /* is run after every test */
{
}

// Weak EC testing
// E1
void test_valid_packet_is_delivered(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, 0x0A ^ 0x14};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(10, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(20, packet_data[1]);
}

// E2
void test_bad_checksum_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (0x0A ^ 0x14) + 1};  /* bad checksum */
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}

void test_noise_before_header_is_ignored(void)
{
    const uint8_t bytes[] = {5, 67, 54, UART_HEADER_BYTE, 1, 42, 42};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(1, packet_len);
    TEST_ASSERT_EQUAL_UINT8(42, packet_data[0]);
}

void test_packet_without_header_is_ignored(void)
{
    const uint8_t bytes[] = {1, 2, 3, 4, 5};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}

void test_header_value_in_data_is_data(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, UART_HEADER_BYTE, 42, 42 ^ UART_HEADER_BYTE};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(UART_HEADER_BYTE, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(42, packet_data[1]);
}

void test_two_packets_in_a_row(void)
{
    const uint8_t bytes[] = {
        UART_HEADER_BYTE, 1, 10, 10,
        UART_HEADER_BYTE, 2, 20, 30, 0x14 ^ 0x1E
    };
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(2, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(20, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(30, packet_data[1]);
}

void test_recovers_after_bad_checksum(void)
{
    const uint8_t bytes[] = {
        UART_HEADER_BYTE, 2, 10, 20, (0x0A ^ 0x14) + 1,   /* bad checksum */
        UART_HEADER_BYTE, 1, 42, 42
    };
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(1, packet_len);
    TEST_ASSERT_EQUAL_UINT8(42, packet_data[0]);
}

void test_checksum_is_reset_between_packets(void)
{
    const uint8_t bytes[] = {
        UART_HEADER_BYTE, 2, 10, 20, 0x0A ^ 0x14,
        UART_HEADER_BYTE, 2, 30, 40, 0x1E ^ 0x28
    };
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(2, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(30, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(40, packet_data[1]);
}

void test_two_parsers_are_independent(void)
{
    parser_t parser_a, parser_b;
    count_a = 0;
    count_b = 0;
    parser_init(&parser_a, on_packet_a);
    parser_init(&parser_b, on_packet_b);

    /* Half a packet to A: header, length 2 and the first data byte */
    parser_feed(&parser_a, UART_HEADER_BYTE);
    parser_feed(&parser_a, 2);
    parser_feed(&parser_a, 0x05);

    /* A whole packet to B in the meantime */
    parser_feed(&parser_b, UART_HEADER_BYTE);
    parser_feed(&parser_b, 1);
    parser_feed(&parser_b, 0x08);
    parser_feed(&parser_b, 0x08);           /* checksum */

    TEST_ASSERT_EQUAL_INT(0, count_a);
    TEST_ASSERT_EQUAL_INT(1, count_b);
    TEST_ASSERT_EQUAL_UINT8(1, len_b);
    TEST_ASSERT_EQUAL_UINT8(0x08, data_b[0]);

    /* The rest of A's packet */
    parser_feed(&parser_a, 0x06);
    parser_feed(&parser_a, 0x05 ^ 0x06);    /* checksum */

    TEST_ASSERT_EQUAL_INT(1, count_a);
    TEST_ASSERT_EQUAL_INT(1, count_b);
    TEST_ASSERT_EQUAL_UINT8(2, len_a);
    TEST_ASSERT_EQUAL_UINT8(0x05, data_a[0]);
    TEST_ASSERT_EQUAL_UINT8(0x06, data_a[1]);
}


// BV testing
void test_length_zero_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 0, 0};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);

    const uint8_t bytes2[] = {UART_HEADER_BYTE, 2, 10, 20, 0x0A ^ 0x14};
    feed(bytes2, sizeof bytes2);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
}

void test_length_one_is_accepted(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 1, 42, 42};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(1, packet_len);
    TEST_ASSERT_EQUAL_UINT8(42, packet_data[0]);
}

void test_max_length_is_accepted(void)
{
    uint8_t expected[UART_MAX_PAYLOAD_LENGTH];
    uint8_t checksum = 0;

    for (uint8_t i = 0; i < UART_MAX_PAYLOAD_LENGTH; i++) {
        expected[i] = i + 1;            /* 1, 2, ..., 64 */
        checksum ^= expected[i];
    }

    parser_feed(&parser, UART_HEADER_BYTE);
    parser_feed(&parser, UART_MAX_PAYLOAD_LENGTH);   /* length */
    feed(expected, sizeof expected);                 /* data */
    parser_feed(&parser, checksum);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(UART_MAX_PAYLOAD_LENGTH, packet_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, packet_data, UART_MAX_PAYLOAD_LENGTH);
}

void test_too_long_packet_is_dropped(void)
{
    uint8_t bytes[UART_MAX_PAYLOAD_LENGTH + 4] = {0};
    bytes[0] = UART_HEADER_BYTE;
    bytes[1] = UART_MAX_PAYLOAD_LENGTH + 1;  /* too long */
    for (uint8_t i = 0; i < UART_MAX_PAYLOAD_LENGTH + 1; i++) {
        bytes[i + 2] = i;
        bytes[UART_MAX_PAYLOAD_LENGTH + 3] ^= i;  /* checksum */
    }
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);

    const uint8_t bytes2[] = {UART_HEADER_BYTE, 2, 10, 20, 0x0A ^ 0x14};
    feed(bytes2, sizeof bytes2);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
}

// With E1 and E2, checksum is tested both just below, just above and exactly at the correct value
void test_checksum_one_below_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (0x0A ^ 0x14) - 1};  /* bad checksum */
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}


// Return value tests
void test_returns_first_byte_after_valid_packet(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, 0x0A ^ 0x14};
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(10, last);
}

void test_returns_SUCCESSFUL_BYTE_in_the_middle_of_a_packet(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20};
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(SUCCESSFUL_BYTE, last);
}

void test_returns_negative_one_after_bad_checksum(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (0x0A ^ 0x14) + 1};  /* bad checksum */
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(-1, last);
}

void test_returns_negative_one_for_invalid_length(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 0};  /* invalid length */
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(-1, last);
}

void test_returns_negative_one_before_header(void)
{
    const uint8_t bytes[] = {1, 2, 3, 4, 5};
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(-1, last);
}

void test_returns_SUCCESSFUL_BYTE_after_header(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE};
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(SUCCESSFUL_BYTE, last);
}

void test_returns_SUCCESSFUL_BYTE_after_length(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2};
    int16_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT16(SUCCESSFUL_BYTE, last);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_packet_is_delivered);
    RUN_TEST(test_bad_checksum_is_dropped);
    RUN_TEST(test_noise_before_header_is_ignored);
    RUN_TEST(test_packet_without_header_is_ignored);
    RUN_TEST(test_header_value_in_data_is_data);
    RUN_TEST(test_two_packets_in_a_row);
    RUN_TEST(test_recovers_after_bad_checksum);
    RUN_TEST(test_checksum_is_reset_between_packets);
    RUN_TEST(test_two_parsers_are_independent);

    RUN_TEST(test_length_zero_is_dropped);
    RUN_TEST(test_length_one_is_accepted);
    RUN_TEST(test_max_length_is_accepted);
    RUN_TEST(test_too_long_packet_is_dropped);
    RUN_TEST(test_checksum_one_below_is_dropped);

    RUN_TEST(test_returns_first_byte_after_valid_packet);
    RUN_TEST(test_returns_SUCCESSFUL_BYTE_in_the_middle_of_a_packet);
    RUN_TEST(test_returns_negative_one_after_bad_checksum);
    RUN_TEST(test_returns_negative_one_for_invalid_length);
    RUN_TEST(test_returns_negative_one_before_header);
    RUN_TEST(test_returns_SUCCESSFUL_BYTE_after_header);
    RUN_TEST(test_returns_SUCCESSFUL_BYTE_after_length);
    return UNITY_END();
}