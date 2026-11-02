#ifndef DECISION_H
#define DECISION_H

#include <stdbool.h>
#include <stdint.h>

/* Beslutslogik (designspec tabell 12, krav 4, 12, 23 och 24). Hinder, stopplinje och
   vilse stoppar bilen. Annars körs bilen och styrs tillbaka mot vägens mitt. */
typedef enum {
    DIRECTIVE_DRIVE,
    DIRECTIVE_STOP
} directive_type;

typedef struct {
    int16_t road_offset_mm;  /* Positivt när bilen är till höger om vägens mitt. */
    bool obstacle;
    bool stop_line;
    bool lost;
} decision_input;

typedef struct {
    directive_type directive;
    int16_t steering;  /* Positivt är höger, negativt är vänster. 0 när bilen står. */
} decision_output;

decision_output decision_decide(const decision_input *input);

#endif
