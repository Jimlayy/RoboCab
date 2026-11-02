#include "message_queue.h"
#include "test.h"

static void check_next(message_queue *queue, uint8_t id)
{
    message m;
    CHECK(message_queue_pop(queue, &m));
    CHECK_EQ(m.id, id);
}

static void fill_with_low(message_queue *queue)
{
    for (uint8_t i = 0; i < MESSAGE_QUEUE_CAPACITY; i++) {
        CHECK(message_queue_push(queue, MESSAGE_PRIORITY_LOW, i));
    }
}

static void test_tom_ko_ger_inget_meddelande(void)
{
    message_queue q;
    message m;
    message_queue_init(&q);

    CHECK(!message_queue_pop(&q, &m));
}

static void test_samma_prioritet_gar_i_ankomstordning(void)
{
    message_queue q;
    message_queue_init(&q);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 1);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 2);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 3);

    check_next(&q, 1);
    check_next(&q, 2);
    check_next(&q, 3);
}

static void test_hinderlarm_gar_fore_koade_lagprioriterade(void)
{
    message_queue q;
    message m;
    message_queue_init(&q);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 1);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 2);
    message_queue_push(&q, MESSAGE_PRIORITY_HIGH, 99);

    CHECK(message_queue_pop(&q, &m));
    CHECK_EQ(m.id, 99);
    CHECK_EQ(m.priority, MESSAGE_PRIORITY_HIGH);
    check_next(&q, 1);
    check_next(&q, 2);
}

static void test_blandad_ordning(void)
{
    message_queue q;
    message_queue_init(&q);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 1);
    message_queue_push(&q, MESSAGE_PRIORITY_HIGH, 2);
    message_queue_push(&q, MESSAGE_PRIORITY_LOW, 3);
    message_queue_push(&q, MESSAGE_PRIORITY_HIGH, 4);

    check_next(&q, 2);
    check_next(&q, 4);
    check_next(&q, 1);
    check_next(&q, 3);
}

static void test_full_ko_avvisar_lagprioriterat(void)
{
    message_queue q;
    message_queue_init(&q);
    fill_with_low(&q);

    CHECK(!message_queue_push(&q, MESSAGE_PRIORITY_LOW, 100));
    check_next(&q, 0);
}

static void test_hinderlarm_far_plats_i_full_ko(void)
{
    message_queue q;
    message_queue_init(&q);
    fill_with_low(&q);

    CHECK(message_queue_push(&q, MESSAGE_PRIORITY_HIGH, 99));
    check_next(&q, 99);
    check_next(&q, 1);  /* Det äldsta lågprioriterade, id 0, kastades. */
}

static void test_ko_full_av_hinderlarm_avvisar_fler(void)
{
    message_queue q;
    message_queue_init(&q);
    for (uint8_t i = 0; i < MESSAGE_QUEUE_CAPACITY; i++) {
        CHECK(message_queue_push(&q, MESSAGE_PRIORITY_HIGH, i));
    }

    CHECK(!message_queue_push(&q, MESSAGE_PRIORITY_HIGH, 100));
    check_next(&q, 0);
}

static void test_ko_gar_att_anvanda_efter_tomning(void)
{
    message_queue q;
    message m;
    message_queue_init(&q);
    fill_with_low(&q);
    for (uint8_t i = 0; i < MESSAGE_QUEUE_CAPACITY; i++) {
        check_next(&q, i);
    }

    CHECK(!message_queue_pop(&q, &m));
    CHECK(message_queue_push(&q, MESSAGE_PRIORITY_LOW, 7));
    check_next(&q, 7);
}

int main(void)
{
    RUN(test_tom_ko_ger_inget_meddelande);
    RUN(test_samma_prioritet_gar_i_ankomstordning);
    RUN(test_hinderlarm_gar_fore_koade_lagprioriterade);
    RUN(test_blandad_ordning);
    RUN(test_full_ko_avvisar_lagprioriterat);
    RUN(test_hinderlarm_far_plats_i_full_ko);
    RUN(test_ko_full_av_hinderlarm_avvisar_fler);
    RUN(test_ko_gar_att_anvanda_efter_tomning);
    return TEST_RESULT();
}
