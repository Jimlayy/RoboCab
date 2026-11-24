#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t adc;
    uint16_t mm;
} distance_point;

typedef enum {
    DISTANCE_OK,
    DISTANCE_TOO_CLOSE,
    DISTANCE_TOO_FAR,
    DISTANCE_NO_TABLE
} distance_result;

distance_result distance_adc_to_mm(uint16_t adc, const distance_point *table, uint8_t count, uint16_t *mm);

// Median av tre mätningar. Tar bort enstaka störda mätningar.
uint16_t distance_median3(uint16_t a, uint16_t b, uint16_t c);

bool distance_obstacle(bool was_obstacle, uint16_t mm, uint16_t threshold_mm, uint16_t hysteresis_mm);

#endif