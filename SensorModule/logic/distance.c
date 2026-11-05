#include "distance.h"

distance_result distance_adc_to_mm(uint16_t adc, const distance_point *table, uint8_t count, uint16_t *mm)
{
    if (count < 2) {
        return DISTANCE_NO_TABLE;
    }

    if (adc > table[0].adc) {
        return DISTANCE_TOO_CLOSE;
    }

    if (adc < table[count - 1].adc) {
        return DISTANCE_TOO_FAR;
    }

    for (uint8_t i = 1; i < count; i++) {
        const distance_point *near = &table[i-1];
        const distance_point *far = &table[i];
        if (adc >= far->adc) {
            if (near->adc == far->adc) {
            *mm = near->mm;
            return DISTANCE_OK;
            }
            uint32_t num = (uint32_t)(near->adc - adc) * (far->mm - near->mm);
            *mm = near->mm + num / (near->adc - far->adc);
            return DISTANCE_OK;
        }
    }
    return DISTANCE_TOO_FAR;
}

// Median av tre mätningar. Tar bort enstaka störda mätningar.
uint16_t distance_median3(uint16_t a, uint16_t b, uint16_t c)
{
    // Medianen är värdet som varken är störst eller minst.
    if ((a >= b && a <= c) || (a <= b && a >= c)) {
        return a;
    }
    if ((b >= a && b <= c) || (b <= a && b >= c)) {
        return b;
    }
    return c;
}

bool distance_obstacle(bool was_obstacle, uint16_t mm, uint16_t threshold_mm, uint16_t hysteresis_mm)
{
    if (was_obstacle) {
        return mm <= (uint32_t)threshold_mm + hysteresis_mm;
    }
    return mm <= threshold_mm;
}