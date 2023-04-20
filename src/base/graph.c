#include "graph.h"

#define INF __UINT32_MAX__

/*********************************** Basics **************************************************/
// Done
struct graph *graph_new(void) {
    struct graph *graph = calloc(1, sizeof(struct graph));
    graph->nodes = vector_init(VECTOR_MIN_SIZE);
    graph->edges = vector_init(VECTOR_MIN_SIZE);
    graph->max_id = 0;
    return graph;
}

// Bugs!
void graph_delete(struct graph *graph) {
    for (unsigned int i = vector_active(graph->nodes); i--; /**/)
        graph_delete_node(graph, vector_slot(graph->nodes, i));

    vector_free(graph->nodes);

    struct graph_edge *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        graph_delete_edge(graph, e->from, e->to);
    }

    vector_free(graph->edges);

    free(graph);
}

// Done
struct graph_node *graph_add_node(struct graph *graph, void *data, void (*del)(void *)) {
    struct graph_node *node = calloc(1, sizeof(struct graph_node));

    node->from = vector_init(VECTOR_MIN_SIZE);
    node->to = vector_init(VECTOR_MIN_SIZE);
    node->id = graph->max_id++;
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

// Done
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
    // Check nodes if exist
    bool f = false, t = false;
    for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
        if (from == vector_slot(graph->nodes, i)) f = true;
        if (to == vector_slot(graph->nodes, i)) t = true;
    }

    if (!f || !t) return NULL;

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

struct graph_edge *graph_find_edge(struct graph *graph, struct graph_node *from, struct graph_node *to) {
    struct graph_edge *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        if (e->from == from && e->to == to) return e;
    }

    return NULL;
}
/*********************************** Views **************************************************/
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
        snprintf(nbuf, sizeof(nbuf), "    n(%d)->n(%d):{%d},\n", e->from->id, e->to->id, e->attr);
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

int graph_number_of_nodes(struct graph *graph) { return graph->nodes->active; }

int graph_number_of_edges(struct graph *graph) { return graph->edges->active; }

/*********************************** Generators **************************************************/

struct graph *graph_gen_trivial() {
    graph_t *g = graph_new();
    graph_add_node(g, 0, 0);
    return g;
}

struct graph *graph_gen_empty(int n) {
    int i;
    graph_t *g = graph_new();
    for (i = 0; i < n; i++) {
        graph_add_node(g, 0, 0);
    }
    return g;
}

struct graph *graph_gen_path(int n) {
    int i;
    graph_t *g = graph_new();
    for (i = 0; i < n; i++) graph_add_node(g, 0, 0);
    for (i = 0; i < g->nodes->active - 1; i++)
        graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, i + 1), 0, NULL);
    return g;
}

struct graph *graph_gen_cycle(int n) {
    struct graph *g = graph_gen_path(n);
    graph_add_edge(g, vector_slot(g->nodes, g->nodes->active - 1), vector_slot(g->nodes, 0), 0, NULL);
    return g;
}

struct graph *graph_gen_star(int n) {
    int i;
    graph_t *g = graph_new();
    graph_node_t *center, *node;
    center = graph_add_node(g, 0, 0);
    for (i = 0; i < n; i++) {
        node = graph_add_node(g, 0, 0);
        graph_add_edge(g, center, node, 0, NULL);
    }
    return g;
}

struct graph *graph_gen_wheel(int n) {
    if (n <= 0) return NULL;
    int i, j;
    struct graph *g = graph_gen_star(n - 1);
    for (i = 1; i < g->nodes->active; i++) {
        j = i == g->nodes->active - 1 ? 1 : i + 1;
        graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, j), 0, NULL);
    }
    return g;
}

struct graph *graph_gen_grid(int m, int n) {
    if (m == 1) return graph_gen_cycle(n);
    if (n == 1) return graph_gen_cycle(m);
    int i, j, k, u, d, l, r;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            k = i * n + j;
            u = k - n;
            d = k + n;
            l = k - 1;
            r = k + 1;
            if (i == 0) u = -1;
            if (i == m - 1) d = -1;
            if (j == 0) l = -1;
            if (j == n - 1) r = -1;

            graph_add_edge(g, )
        }
    }
}

struct graph *graph_gen_complete(int n) {
    int i, j;
    graph_t *g = graph_new();
    for (i = 0; i < n; i++) graph_add_node(g, 0, 0);

    for (i = 0; i < g->nodes->active; i++) {
        for (j = 0; j < g->nodes->active; j++) {
            if (i == j) continue;
            graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, j), 0, NULL);
        }
    }
    return g;
}

struct graph *graph_gen_binomial_tree(int order) {}  // n = 2^order

struct graph *graph_gen_random(int n) {}
struct graph *graph_gen_random_tree(int n) {}

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

static int min_dist(int dist[], bool sptSet[], int n) {
    unsigned int min = INF, min_index;
    for (int v = 0; v < n; v++) {
        if (sptSet[v] == false && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }

    return min_index;
}

void graph_shortest_path(struct graph *graph, struct graph_node *from) {
    int nn = graph->nodes->active;
    int src = -1;  // from index
    for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
        if (vector_slot(graph->nodes, i) == from) {
            src = i;
            break;
        }
    }
    if (src == -1) return;

    unsigned int dist[nn];
    bool sptSet[nn];
    int path[nn];

    // Initial all distances as INF and stpSet[] as false
    for (int i = 0; i < nn; i++) {
        dist[i] = INF;
        sptSet[i] = false;
        path[i] = -1;
    }
    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < nn - 1; count++) {
        // Pick the minimum distance vertex from the set of vertices not yet processed.
        // u is always equal to src in the first iteration
        int u = min_dist(dist, sptSet, nn);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the picked vertex
        for (int v = 0; v < nn; v++) {
            // Update dist[v] only if is not in sptSet, there is an edge from u to v, and total weight of path from
            // to v through u is smaller than current value of dist[v]
            struct graph_edge *e = graph_find_edge(graph, vector_slot(graph->nodes, u), vector_slot(graph->nodes, v));
            unsigned int weight = e ? (int)e->attr : 0;
            if (!sptSet[v] && e && dist[u] != INF && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                path[v] = u;
            }
        }
    }

    // print solution
    struct graph_node *n, *p;
    for (int i = 0; i < nn; i++) {
        if (path[i] == -1) continue;
        n = vector_slot(graph->nodes, i);
        p = vector_slot(graph->nodes, path[i]);
        printf("n(%d) -> n(%d): distance %d via n(%d)\n", from->id, n->id, dist[i], p->id);
    }
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