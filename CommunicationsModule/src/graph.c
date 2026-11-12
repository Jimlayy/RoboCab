#include <stdbool.h>
#include <float.h>
#include "graph.h"

static dir_t opposite(dir_t d)
{
    return (dir_t)((d + 2) % 4);   // N<->S, E<->W
}

static bool valid_node(const graph_t *g, int i)
{
    return i >= 0 && i < g->num_nodes;
}

int graph_add_edge(graph_t *g, int a, int b, float length, dir_t dir_ab)
{
    if (!valid_node(g, a) || !valid_node(g, b) || a == b)
        return -1;

    node_t *na = &g->nodes[a], *nb = &g->nodes[b];
    if (na->num_edges >= MAX_EDGES_PER_NODE || nb->num_edges >= MAX_EDGES_PER_NODE)
        return -1;

    na->edges[na->num_edges++] = (edge_t){ .to = b, .length = length, .leaves_as = dir_ab };
    nb->edges[nb->num_edges++] = (edge_t){ .to = a, .length = length, .leaves_as = opposite(dir_ab) };
    return 0;
}

int construct_graph(graph_t *g)
{
    // Replace with the actual track (later: read from a file)
    *g = (graph_t){ .num_nodes = 5 };              // start from an empty graph
    for (int i = 0; i < g->num_nodes; i++)
        g->nodes[i].id = i;
    g->nodes[2].type = NODE_INTERSECTION;

    int err = 0;
    err |= graph_add_edge(g, 0, 1, 100, DIR_WEST);
    err |= graph_add_edge(g, 1, 2,  50, DIR_SOUTH);
    err |= graph_add_edge(g, 0, 3,  40, DIR_SOUTH);
    err |= graph_add_edge(g, 3, 2,  60, DIR_WEST);
    err |= graph_add_edge(g, 2, 4,  80, DIR_WEST);
    return err ? -1 : 0;
}

int shortest_path_no_uturn(const graph_t *g, int start_prev, int start,
                           int goal, int goal_prev, int route[MAX_ROUTE])
{
    static float dist[MAX_NODES][MAX_NODES];     // dist[prev][node]
    static int   back[MAX_NODES][MAX_NODES];     // previous state's prev
    static bool  done[MAX_NODES][MAX_NODES];
    int n = g->num_nodes;

    if (!valid_node(g, start_prev) || !valid_node(g, start) || !valid_node(g, goal))
        return -1;

    for (int p = 0; p < n; p++)
        for (int u = 0; u < n; u++) {
            dist[p][u] = FLT_MAX;
            back[p][u] = NO_NODE;
            done[p][u] = false;
        }
    dist[start_prev][start] = 0;

    int end_p = NO_NODE, end_u = NO_NODE;

    for (;;) {
        int bp = -1, bu = -1;                    // closest unfinished state
        for (int p = 0; p < n; p++)
            for (int u = 0; u < n; u++)
                if (!done[p][u] && dist[p][u] < FLT_MAX &&
                    (bp < 0 || dist[p][u] < dist[bp][bu])) { bp = p; bu = u; }
        if (bp < 0) break;                       // nothing left: unreachable
        done[bp][bu] = true;

        if (bu == goal && (goal_prev == NO_NODE || bp == goal_prev)) {
            end_p = bp; end_u = bu;
            break;
        }

        const node_t *node = &g->nodes[bu];
        for (int e = 0; e < node->num_edges; e++) {
            int v = node->edges[e].to;
            if (v == bp)                         // the no-U-turn rule
                continue;
            float nd = dist[bp][bu] + node->edges[e].length;
            if (nd < dist[bu][v]) {
                dist[bu][v] = nd;
                back[bu][v] = bp;
            }
        }
    }
    if (end_u == NO_NODE) return -1;

    int tmp[MAX_ROUTE], k = 0;                   // walk back through the states
    int p = end_p, u = end_u;
    while (!(p == start_prev && u == start)) {
        tmp[k++] = u;
        int pp = back[p][u];
        u = p;
        p = pp;
    }
    tmp[k++] = start;
    for (int i = 0; i < k; i++)                  // reverse: start .. goal
        route[i] = tmp[k - 1 - i];
    return k;
}