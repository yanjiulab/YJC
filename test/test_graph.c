#include "graph.h"
#include "stdlib.h"
#include "test.h"

void test_graph() {
    graph_t *g = graph_new(DiGraph);
    graph_edge_t *e;

    int i;
    struct graph_node *gn[10];
    for (i = 0; i < 10; i++) {
        gn[i] = graph_add_node(g, i, 0);
    }

    // graph_delete_node(g, gn[3]);
    // graph_delete_node(g, gn[4]);

    // printf("active:%d\n", g->nodes->active);
    // printf("alloced:%d\n", g->nodes->alloced);
    // printf("count:%d\n", g->nodes->count);

    graph_add_edge(g, gn[0], gn[1], (void *)1, NULL);
    graph_add_edge(g, gn[1], gn[2], (void *)1, NULL);
    graph_add_edge(g, gn[2], gn[3], (void *)1, NULL);
    graph_add_edge(g, gn[3], gn[4], (void *)1, NULL);
    graph_add_edge(g, gn[4], gn[5], (void *)1, NULL);
    graph_add_edge(g, gn[5], gn[6], (void *)1, NULL);
    graph_add_edge(g, gn[6], gn[7], (void *)1, NULL);
    graph_add_edge(g, gn[7], gn[8], (void *)1, NULL);
    graph_add_edge(g, gn[8], gn[9], (void *)1, NULL);
    graph_add_edge(g, gn[9], gn[0], (void *)1, NULL);

    // graph_dump_nodes(g);
    // graph_dump_edges(g);
    // printf("nodes: %d\n", graph_number_of_nodes(g));
    // printf("edges: %d\n", graph_number_of_edges(g));
    graph_shortest_path(g, gn[5]);

    graph_reverse(g, false);
    graph_dump(g);
    graph_shortest_path(g, gn[5]);

    graph_delete(g);

    graph_type_t t = DiGraph;
    // g = graph_gen_trivial(t);
    // graph_dump(g);
    // g = graph_gen_empty(5, t);
    // graph_dump(g);
    g = graph_gen_path(5, t);
    graph_dump(g);
    graph_reverse(g, false);
    graph_dump(g);
    // g = graph_gen_cycle(5, t);
    // graph_dump(g);
    // g = graph_gen_star(5, t);
    // graph_dump(g);
    // g = graph_gen_wheel(6, t);
    // graph_dump(g);
    // g = graph_gen_grid(3, 3, t);
    // graph_dump(g);
    // g = graph_gen_complete(5, t);
    // graph_dump(g);

    // graph_shortest_path(g, gn[5]);

    // rand()
}