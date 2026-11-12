#include <stdio.h>
#include "graph.h"

static void show(const char *label, int n, const int *route)
{
    printf("%-36s", label);
    if (n < 0) { printf("no route\n"); return; }
    for (int i = 0; i < n; i++)
        printf("%d%s", route[i], i + 1 < n ? " -> " : "\n");
}

/*int main(void)
{
    static graph_t g;
    if (construct_graph(&g) != 0) {
        fprintf(stderr, "construct_graph failed\n");
        return 1;
    }

    int route[MAX_ROUTE];
    show("Car 3->0, goal 4:",           shortest_path_no_uturn(&g, 3, 0, 4, NO_NODE, route), route);
    show("Car 1->0, goal 4:",           shortest_path_no_uturn(&g, 1, 0, 4, NO_NODE, route), route);
    show("Car 3->0, goal 3:",           shortest_path_no_uturn(&g, 3, 0, 3, NO_NODE, route), route);
    show("Car 3->0, goal 2 from 1:",    shortest_path_no_uturn(&g, 3, 0, 2, 1,       route), route);
    show("Car 3->0, goal 2 from 3:",    shortest_path_no_uturn(&g, 3, 0, 2, 3,       route), route);
    return 0;
}*/