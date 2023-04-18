#include "graph.h"

static unsigned int nid = 0;

/*********************************** Basics **************************************************/
// Done
struct graph *graph_new(void) {
    struct graph *graph = calloc(1, sizeof(struct graph));
    graph->nodes = vector_init(VECTOR_MIN_SIZE);
    graph->edges = vector_init(VECTOR_MIN_SIZE);

    return graph;
}

// Done
void graph_delete(struct graph *graph) {
    for (unsigned int i = vector_active(graph->nodes); i--; /**/)
        graph_delete_node(graph, vector_slot(graph->nodes, i));

    vector_free(graph->nodes);

    struct graph_edge *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        graph_delete_edge(graph, e->from, e->to);
    }

    vector_free(graph->nodes);
    vector_free(graph->edges);

    free(graph);
}

void graph_dump_nodes(struct graph *graph) {
    char nbuf[64];
    graph_node_t *n;
    printf("  nodes = {");
    for (unsigned int i = 0; i < vector_active(graph->nodes); i++) {
        n = vector_slot(graph->nodes, i);
        snprintf(nbuf, sizeof(nbuf), "%s%d:{%p}", i ? ", " : "", n->id, n->data);
        printf("%s", nbuf);
    }
    printf("}\n");
}

void graph_dump_edges(struct graph *graph) {
    char nbuf[64];
    graph_edge_t *e;
    printf("  edges = {\n");
    for (unsigned int i = 0; i < vector_active(graph->edges); i++) {
        e = vector_slot(graph->edges, i);
        snprintf(nbuf, sizeof(nbuf), "    n(%d)->n(%d):{%p},\n", e->from->id, e->to->id, e->attr);
        printf("%s", nbuf);
    }
    printf("  }\n");
} 

void graph_dump(struct graph *graph) {
    printf("DiGraph = {\n");
    graph_dump_nodes(graph);
    graph_dump_edges(graph);
    printf("}\n");
}

// Done
struct graph_node *graph_add_node(struct graph *graph, void *data, void (*del)(void *)) {
    struct graph_node *node = calloc(1, sizeof(struct graph_node));

    node->from = vector_init(VECTOR_MIN_SIZE);
    node->to = vector_init(VECTOR_MIN_SIZE);
    node->id = nid++;
    node->data = data;
    node->del = del;

    vector_set(graph->nodes, node);

    return node;
}

// Done
static void graph_vector_remove(vector v, unsigned int ix) {
    if (ix >= v->active) return;

    /* v->active is guaranteed >= 1 because ix can't be lower than 0
     * and v->active is > ix. */
    v->active--;
    /* if ix == v->active--, we set the item to itself, then to NULL...
     * still correct, no check necessary. */
    v->index[ix] = v->index[v->active];
    v->index[v->active] = NULL;
}

// TODO 需要同时删除链路，need remove age
void graph_delete_node(struct graph *graph, struct graph_node *node) {
    if (!node) return;

    // an adjacent node
    struct graph_node *adj;

    // remove all edges from other nodes to us
    for (unsigned int i = vector_active(node->from); i--; /**/) {
        adj = vector_slot(node->from, i);
        graph_delete_edge(graph, adj, node);
    }

    // remove all edges from us to other nodes
    for (unsigned int i = vector_active(node->to); i--; /**/) {
        adj = vector_slot(node->to, i);
        graph_delete_edge(graph, node, adj);
    }

    // if there is a deletion callback, call it
    if (node->del && node->data) (*node->del)(node->data);

    // free adjacency lists
    vector_free(node->to);
    vector_free(node->from);

    // remove node from graph->nodes
    for (unsigned int i = vector_active(graph->nodes); i--; /**/)
        if (vector_slot(graph->nodes, i) == node) {
            graph_vector_remove(graph->nodes, i);
            break;
        }

    // free the node itself
    free(node);
}

// Done
struct graph_edge *graph_add_edge(struct graph *graph, struct graph_node *from, struct graph_node *to, void *attr,
                                  void (*del)(void *)) {
    struct graph_edge *edge = calloc(1, sizeof(struct graph_edge));
    edge->from = from;
    edge->to = to;
    edge->attr = attr;
    edge->del = del;

    vector_set(graph->edges, edge);
    vector_set(from->to, to);
    vector_set(to->from, from);
    return edge;
}

// Done
void graph_delete_edge(struct graph *graph, struct graph_node *from, struct graph_node *to) {
    struct graph_edge *e;

    // remove edge from graph->edges
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        if (e->from == from && e->to == to) {
            graph_vector_remove(graph->edges, i);
        }
    }

    // remove from from to->from
    for (unsigned int i = vector_active(to->from); i--; /**/)
        if (vector_slot(to->from, i) == from) {
            graph_vector_remove(to->from, i);
            break;
        }
    // remove to from from->to
    for (unsigned int i = vector_active(from->to); i--; /**/)
        if (vector_slot(from->to, i) == to) {
            graph_vector_remove(from->to, i);
            break;
        }
}

// Done (need improve)
struct graph_node *graph_find_node(struct graph *graph, void *data) {
    struct graph_node *g;

    for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
        g = vector_slot(graph->nodes, i);
        if (g->data == data) return g;
    }

    return NULL;
}

bool graph_has_edge(struct graph *graph, struct graph_node *from, struct graph_node *to) {
    for (unsigned int i = vector_active(from->to); i--; /**/)
        if (vector_slot(from->to, i) == to) return true;

    // struct graph_edge *e;
    // for (unsigned int i = vector_active(graph->edges); i--; /**/) {
    //     e = vector_slot(graph->edges, i);
    //     if (e->from == from && e->to == to) return true;
    // }

    return false;
}

/*********************************** Algorithms **************************************************/
static void _graph_dfs(struct graph *graph, struct graph_node *start, vector visited,
                       void (*dfs_cb)(struct graph_node *, void *), void *arg) {
    /* check that we have not visited this node */
    for (unsigned int i = 0; i < vector_active(visited); i++) {
        if (start == vector_slot(visited, i)) return;
    }

    /* put this node in visited stack */
    vector_ensure(visited, vector_active(visited));
    vector_set_index(visited, vector_active(visited), start);

    /* callback */
    dfs_cb(start, arg);

    /* recurse into children */
    for (unsigned int i = vector_active(start->to); i--; /**/) {
        struct graph_node *c = vector_slot(start->to, i);

        _graph_dfs(graph, c, visited, dfs_cb, arg);
    }
}

void graph_dfs(struct graph *graph, struct graph_node *start, void (*dfs_cb)(struct graph_node *, void *), void *arg) {
    vector visited = vector_init(VECTOR_MIN_SIZE);

    _graph_dfs(graph, start, visited, dfs_cb, arg);
    vector_free(visited);
}

void graph_dump_dot_default_print_cb(struct graph_node *gn) {
    char nbuf[64];

    for (unsigned int i = 0; i < vector_active(gn->to); i++) {
        struct graph_node *adj = vector_slot(gn->to, i);
        snprintf(nbuf, sizeof(nbuf), "    n(%d) -> n(%d);\n", gn->id, adj->id);
        printf("%s", nbuf);
    }
}

void graph_shortest_path(struct graph *graph, struct graph_node *from) {
    // int nn = graph->nodes->active;
    // int src = graph->nodes.

    // int dist[nn];
    // bool sptSet[nn];
    // int path[nn] = {0};

    // // Initial all distances as INF and stpSet[] as false
    // for (int i = 0; i < nn; i++) {
    //     dist[i] = 0xffffff;
    //     sptSet[i] = false;
    //     path[i] = -1;
    // }
    // // Distance of source vertex from itself is always 0
    // dist[src] = 0;

    // // Find shortest path for all vertices
    // for (int count = 0; count < nn - 1; count++) {
    //     // Pick the minimum distance vertex from the set of vertices not yet processed.
    //     // u is always equal to src in the first iteration
    //     int u = min_dist(dist, sptSet);

    //     // Mark the picked vertex as processed
    //     sptSet[u] = true;

    //     // Update dist value of the adjacent vertices of the picked vertex
    //     for (int v = 0; v < nn; v++) {
    //         // Update dist[v] only if is not in sptSet, there is an edge from u to v, and total weight of path from src
    //         // to v through u is smaller than current value of dist[v]
    //         if (!sptSet[v] && ISCONN(u, v) && dist[u] != LINK_BREAK && dist[u] + 1 < dist[v]) {
    //             dist[v] = dist[u] + 1;
    //             path[v] = u;
    //         }
    //     }
    // }
}


// void graph_dump_dot_default_print_cb(struct graph_node *gn, struct buffer *buf)
// {
// 	char nbuf[64];

// 	for (unsigned int i = 0; i < vector_active(gn->to); i++) {
// 		struct graph_node *adj = vector_slot(gn->to, i);

// 		snprintf(nbuf, sizeof(nbuf), "    n%p -> n%p;\n", gn, adj);
// 		buffer_putstr(buf, nbuf);
// 	}
// }

// char *graph_dump_dot(struct graph *graph, struct graph_node *start,
// 		     void (*pcb)(struct graph_node *, struct buffer *))
// {
// 	struct buffer *buf = buffer_new(0);
// 	char *ret;

// 	pcb = (pcb) ? pcb : graph_dump_dot_default_print_cb;
// 	buffer_putstr(buf, "digraph {\n");

// 	graph_dfs(graph, start, (void (*)(struct graph_node *, void *))pcb,
// 		  buf);

// 	buffer_putstr(buf, "}\n");

// 	ret = buffer_getstr(buf);
// 	buffer_free(buf);

// 	return ret;
// }