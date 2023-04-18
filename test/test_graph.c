#include "graph.h"
#include "stdlib.h"
#include "test.h"

// node_attr
// edge

void test_graph() {
    graph_t *g = graph_new();

    int i;
    struct graph_node *gn[10];
    for (i = 0; i < 10; i++) {
        gn[i] = graph_add_node(g, i, 0);
    }
    graph_add_edge(g, gn[0], gn[1], 1, NULL);
    graph_add_edge(g, gn[1], gn[2], 1, NULL);
    graph_add_edge(g, gn[2], gn[3], 1, NULL);
    graph_add_edge(g, gn[3], gn[4], 1, NULL);
    graph_add_edge(g, gn[4], gn[5], 1, NULL);
    graph_add_edge(g, gn[5], gn[6], 1, NULL);
    graph_add_edge(g, gn[6], gn[7], 1, NULL);
    graph_add_edge(g, gn[7], gn[8], 1, NULL);
    graph_add_edge(g, gn[8], gn[9], 1, NULL);
    graph_add_edge(g, gn[9], gn[0], 1, NULL);

    for (unsigned int i = vector_active(g->nodes); i--; /**/) {
        graph_dump_dot_default_print_cb(vector_slot(g->nodes, i));
    }

    // graph_dump_nodes(g);
    // graph_dump_edges(g);
    graph_dump(g);

    printf("%d\n", g->nodes->active);
    printf("%d\n", g->nodes->alloced);
    printf("%d\n", g->nodes->count);
    // rand()
}