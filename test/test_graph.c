#include "graph.h"
#include "stdlib.h"
#include "test.h"

void test_graph() {
    graph_t *g = graph_new();
    int node_data = 10;

    int i;
    struct graph_node *gn[10];
    for (i = 0; i < 10; i++) {
        gn[i] = graph_new_node(g, &i, 0);
    }
    graph_add_edge(gn[0], gn[1]);
    graph_add_edge(gn[1], gn[2]);
    graph_add_edge(gn[2], gn[3]);
    graph_add_edge(gn[3], gn[4]);
    graph_add_edge(gn[4], gn[5]);
    graph_add_edge(gn[5], gn[6]);
    graph_add_edge(gn[6], gn[7]);
    graph_add_edge(gn[7], gn[8]);
    graph_add_edge(gn[8], gn[9]);
    graph_add_edge(gn[9], gn[0]);


    for (unsigned int i = vector_active(g->nodes); i--; /**/) {
        graph_dump_dot_default_print_cb(vector_slot(g->nodes, i));
    }
    // rand()
}