#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

/* Prioritetskö för meddelanden (designspec tabell 12, krav 12 och 23).
   Hög prioritet går alltid först, och samma prioritet går i ankomstordning.
   Är kön full kastas det äldsta lågprioriterade meddelandet för ett högprioriterat,
   så att ett hinderlarm inte går förlorat. */
#define MESSAGE_QUEUE_CAPACITY 8

typedef enum {
    MESSAGE_PRIORITY_LOW,
    MESSAGE_PRIORITY_HIGH
} message_priority;

typedef struct {
    message_priority priority;
    uint8_t id;
} message;

typedef struct {
    uint8_t count;
    message items[MESSAGE_QUEUE_CAPACITY];
} message_queue;

void message_queue_init(message_queue *queue);

/* Returnerar false om meddelandet inte fick plats. */
bool message_queue_push(message_queue *queue, message_priority priority, uint8_t id);

/* Returnerar false om kön är tom. */
bool message_queue_pop(message_queue *queue, message *out);

#endif
