#ifndef ROUTE_H
#define ROUTE_H

#include <stdbool.h>
#include <stdint.h>

/* Ruttplanering (designspec 4.7). Kanterna är oriktade. Kortaste längd går först,
   och vid lika långa vägar väljs den med minst antal svängar.
   Bilen får aldrig välja noden den just kom från, så sökningen behöver ett
   tillstånd med både nod och föregående nod. Samma nod kan då passeras två gånger. */
#define ROUTE_MAX_NODES 32
#define ROUTE_MAX_EDGES 64
#define ROUTE_MAX_PATH 64
#define ROUTE_ANY 0xFF

typedef struct {
    uint8_t a;
    uint8_t b;
    uint16_t length;
} route_edge;

typedef struct {
    uint8_t from;
    uint8_t via;
    uint8_t to;
} route_passage;

typedef struct {
    uint8_t node_count;
    uint8_t edge_count;
    uint8_t straight_count;
    route_edge edges[ROUTE_MAX_EDGES];
    route_passage straights[ROUTE_MAX_EDGES];
} route_graph;

typedef struct {
    uint8_t node_count;
    uint32_t length;
    uint8_t nodes[ROUTE_MAX_PATH];
} route_path;

void route_init(route_graph *graph, uint8_t node_count);

/* Returnerar false om en nod inte finns eller om grafen är full. */
bool route_add_edge(route_graph *graph, uint8_t a, uint8_t b, uint16_t length);

/* Markerar att from-via-to går rakt fram, i båda riktningarna.
   Alla andra passager räknas som en sväng. */
bool route_set_straight(route_graph *graph, uint8_t from, uint8_t via, uint8_t to);

/* Söker från start till goal. Rutten innehåller både start och goal.
   came_from: noden bilen kom från, eller ROUTE_ANY. Första steget får inte gå dit.
   arrive_from: noden bilen måste ankomma från till goal, eller ROUTE_ANY.
   Returnerar false om ingen väg finns. */
bool route_find(const route_graph *graph, uint8_t start, uint8_t came_from,
                uint8_t goal, uint8_t arrive_from, route_path *path);

#endif
