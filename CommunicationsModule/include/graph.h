#ifndef GRAPH_H
#define GRAPH_H

#define MAX_NODES          32
#define MAX_EDGES_PER_NODE 4                       // a 4-way intersection at most
#define MAX_ROUTE          (MAX_NODES * MAX_NODES) // size needed for a route array
#define NO_NODE            (-1)

typedef enum { NODE_STOPLINE, NODE_INTERSECTION, NODE_PICKUP } node_type_t;
typedef enum { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST } dir_t;

typedef struct {
    int   to;          // index of the neighbouring node
    float length;      // segment length in cm
    dir_t leaves_as;   // the direction the car drives when it takes this edge
} edge_t;

typedef struct {
    int         id;
    node_type_t type;
    int         num_edges;
    edge_t      edges[MAX_EDGES_PER_NODE];
} node_t;

typedef struct {
    int    num_nodes;
    node_t nodes[MAX_NODES];
} graph_t;

/* Adds a road between a and b. dir_ab is the direction you drive going a -> b;
 * the way back (b -> a) gets the opposite direction automatically.
 * Returns 0 on success, -1 if a node index is invalid or a node is full. */
int graph_add_edge(graph_t *g, int a, int b, float length, dir_t dir_ab);

/* Fills g with the track. Returns 0 on success, -1 on error. */
int construct_graph(graph_t *g);

/* Shortest path where the car may never turn back to the node it just came from.
 *   start_prev, start : the car is driving from start_prev towards start
 *   goal              : node to reach
 *   goal_prev         : node the car must arrive FROM (pickup direction),
 *                       or NO_NODE if any direction is fine
 *   route[]           : receives start .. goal, must have room for MAX_ROUTE
 * Returns the number of nodes in route, or -1 if no allowed route exists. */
int shortest_path_no_uturn(const graph_t *g, int start_prev, int start,
                           int goal, int goal_prev, int route[MAX_ROUTE]);

#endif