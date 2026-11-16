#include "unity.h"
#include "parser.h"
#include <string.h>
#include <stddef.h>

// Result variables
static int packet_count;
static uint8_t packet_len;
static uint8_t packet_data[UART_MAX_PAYLOAD_LENGTH];

// Store the result values
static void on_packet(const uint8_t *data, uint8_t len)
{
    packet_count++;
    packet_len = len;
    for (uint8_t i = 0; i < len; i++) {
        packet_data[i] = data[i];
    }
}

static parser_t parser;

// Helper function, feed a sequence of bytes to the parser and return the result of the last byte
static uint8_t feed(const uint8_t *bytes, size_t count)
{
    uint8_t last = 0;
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
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, 10 ^ 20};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(10, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(20, packet_data[1]);
}

// E2
void test_bad_checksum_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (10 ^ 20) + 1};  /* bad checksum */
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}

void test_noise_before_header_is_ignored(void)
{
    const uint8_t bytes[] = {5, 67, 420, UART_HEADER_BYTE, 1, 42, 42};
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
        UART_HEADER_BYTE, 2, 20, 30, 20 ^ 30
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
        UART_HEADER_BYTE, 2, 10, 20, (10 ^ 20) + 1,   /* bad checksum */
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
        UART_HEADER_BYTE, 2, 10, 20, 10 ^ 20,
        UART_HEADER_BYTE, 2, 30, 40, 30 ^ 40
    };
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(2, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(30, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(40, packet_data[1]);
}

void test_two_parsers_are_independent(void)
{
    parser_t parser1, parser2;
    uint8_t packet_count1 = 0, packet_count2 = 0;

    void on_packet1(const uint8_t *data, uint8_t len) { packet_count1++; }
    void on_packet2(const uint8_t *data, uint8_t len) { packet_count2++; }

    parser_init(&parser1, on_packet1);
    parser_init(&parser2, on_packet2);

    const uint8_t bytes[] = {UART_HEADER_BYTE, 1, 42, 42};
    feed(bytes, sizeof bytes);  /* feed to global parser */

    TEST_ASSERT_EQUAL_INT(0, packet_count1);
    TEST_ASSERT_EQUAL_INT(0, packet_count2);

    for (size_t i = 0; i < sizeof bytes; i++) {
        parser_feed(&parser1, bytes[i]);
        parser_feed(&parser2, bytes[i]);
    }

    TEST_ASSERT_EQUAL_INT(1, packet_count1);
    TEST_ASSERT_EQUAL_INT(1, packet_count2);
}


// BV testing
void test_length_zero_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 0, 0};
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
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
    uint8_t bytes[UART_MAX_PAYLOAD_LENGTH + 4];
    bytes[0] = UART_HEADER_BYTE;
    bytes[1] = UART_MAX_PAYLOAD_LENGTH + 1;  /* too long */
    for (uint8_t i = 0; i < UART_MAX_PAYLOAD_LENGTH + 1; i++) {
        bytes[i + 2] = i;
        bytes[UART_MAX_PAYLOAD_LENGTH + 3] ^= i;  /* checksum */
    }
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}

// With E1 and E2, checksum is tested both just below, just above and exactly at the correct value
void test_checksum_one_below_is_dropped(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (10 ^ 20) - 1};  /* bad checksum */
    feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_INT(0, packet_count);
}


// Return value tests
void test_returns_message_id_after_valid_packet(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, 10 ^ 20};
    uint8_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_UINT8(10, last);
}

void test_returns_zero_in_the_middle_of_a_packet(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10};
    uint8_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_UINT8(0, last);
}

void test_returns_zero_after_bad_checksum(void)
{
    const uint8_t bytes[] = {UART_HEADER_BYTE, 2, 10, 20, (10 ^ 20) + 1};  /* bad checksum */
    uint8_t last = feed(bytes, sizeof bytes);

    TEST_ASSERT_EQUAL_UINT8(0, last);
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

    RUN_TEST(test_returns_message_id_after_valid_packet);
    RUN_TEST(test_returns_zero_in_the_middle_of_a_packet);
    RUN_TEST(test_returns_zero_after_bad_checksum);
    return UNITY_END();
}