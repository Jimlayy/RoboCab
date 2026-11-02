#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <stdint.h>

/* Lokalisering (designspec 4.7): rätt nodtyp inom intervallet runt den väntade noden
   godkänns. Fel nodtyp i intervallet, eller ingen nod när intervallet har passerats,
   betyder att bilen är vilse. Detekteringar före intervallet ignoreras. */
typedef enum {
    NODE_NONE,
    NODE_STOP_LINE,
    NODE_INTERSECTION
} node_type;

typedef enum {
    LOCALIZATION_WAITING,
    LOCALIZATION_NODE_ACCEPTED,
    LOCALIZATION_LOST
} localization_result;

/* distance_to_node_mm: odometrins avstånd till väntad nod, negativt efter passage. */
localization_result localization_check(node_type expected, node_type detected,
                                       int32_t distance_to_node_mm,
                                       int32_t tolerance_mm);

#endif
