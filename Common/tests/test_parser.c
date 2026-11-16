#include "unity.h"
#include "parser.h"
#include <string.h>

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

void test_valid_packet_is_delivered(void)
{
    parser_feed(&parser, UART_HEADER_BYTE);
    parser_feed(&parser, 2);           /* length */
    parser_feed(&parser, 10);          /* data byte 1 */
    parser_feed(&parser, 20);          /* data byte 2 */
    parser_feed(&parser, 10 ^ 20);     /* checksum */

    TEST_ASSERT_EQUAL_INT(1, packet_count);
    TEST_ASSERT_EQUAL_UINT8(2, packet_len);
    TEST_ASSERT_EQUAL_UINT8(10, packet_data[0]);
    TEST_ASSERT_EQUAL_UINT8(20, packet_data[1]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_packet_is_delivered);
    return UNITY_END();
}