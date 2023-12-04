#include "graph.h"

#define INF __UINT32_MAX__

/*********************************** Basics **************************************************/
// Done
graph_t *graph_new(graph_type_t t) {
    graph_t *graph = calloc(1, sizeof(graph_t));
    graph->nodes = vector_init(VECTOR_MIN_SIZE);
    graph->edges = vector_init(VECTOR_MIN_SIZE);
    graph->max_id = 0;
    graph->type = t;
    return graph;
}

// Done
void graph_delete(graph_t *graph) {
    for (unsigned int i = vector_active(graph->nodes); i--; /**/)
        graph_delete_node(graph, vector_slot(graph->nodes, i));

    vector_free(graph->nodes);

    graph_edge_t *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        graph_delete_edge(graph, e->from, e->to);
    }

    vector_free(graph->edges);

    free(graph);
}

// Done
graph_node_t *graph_add_node(graph_t *graph, void *data, void (*del)(void *)) {
    graph_node_t *node = calloc(1, sizeof(graph_node_t));

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
void graph_delete_node(graph_t *graph, graph_node_t *node) {
    if (!node) return;

    // an adjacent node
    graph_node_t *adj;

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
static graph_edge_t *_graph_add_edge(graph_t *graph, graph_node_t *from, graph_node_t *to, void *attr,
                                     void (*del)(void *)) {
    graph_edge_t *edge = calloc(1, sizeof(graph_edge_t));
    edge->from = from;
    edge->to = to;
    edge->attr = attr;
    edge->del = del;

    vector_set(graph->edges, edge);
    vector_set(from->to, to);
    vector_set(to->from, from);
    return edge;
}

graph_edge_t *graph_add_edge(graph_t *graph, graph_node_t *from, graph_node_t *to, void *attr, void (*del)(void *)) {
    graph_edge_t *e = NULL;

    // Check nodes if exist
    bool f = false, t = false;
    for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
        if (from == vector_slot(graph->nodes, i)) f = true;
        if (to == vector_slot(graph->nodes, i)) t = true;
    }
    if (!f || !t) return e;

    switch (graph->type) {
        case Graph:
            if (!graph_has_edge(graph, from, to)) e = _graph_add_edge(graph, from, to, attr, del);
            if (!graph_has_edge(graph, to, from)) e = _graph_add_edge(graph, to, from, attr, del);
            break;
        case DiGraph:
            if (!graph_has_edge(graph, from, to)) e = _graph_add_edge(graph, from, to, attr, del);
            break;
        case MultiGraph:
            e = _graph_add_edge(graph, from, to, attr, del);
            e = _graph_add_edge(graph, to, from, attr, del);
            break;
        case MultiDiGraph:
            e = _graph_add_edge(graph, from, to, attr, del);
            break;
        default:
            if (!graph_has_edge(graph, from, to)) {
                e = _graph_add_edge(graph, from, to, attr, del);
            }
            break;
    }
    return e;
}

static void _graph_delete_edge(graph_t *graph, graph_node_t *from, graph_node_t *to) {
    graph_edge_t *e;

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

    // if there is a deletion callback, call it
    if (e->del && e->attr) (*e->del)(e->attr);
}

// How about multiple edge deletion?
void graph_delete_edge(graph_t *graph, graph_node_t *from, graph_node_t *to) {
    switch (graph->type) {
        case Graph:
            _graph_delete_edge(graph, from, to);
            _graph_delete_edge(graph, to, from);
            break;
        case DiGraph:
            _graph_delete_edge(graph, from, to);
            break;
        case MultiGraph:
            _graph_delete_edge(graph, from, to);
            _graph_delete_edge(graph, to, from);
            break;
        case MultiDiGraph:
            _graph_delete_edge(graph, from, to);
            break;
        default:
            _graph_delete_edge(graph, from, to);
            break;
    }
}

// Done (need improve)
graph_node_t *graph_find_node(graph_t *graph, void *data) {
    graph_node_t *g;

    for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
        g = vector_slot(graph->nodes, i);
        if (g->data == data) return g;
    }

    return NULL;
}

bool graph_has_edge(graph_t *graph, graph_node_t *from, graph_node_t *to) {
    graph_edge_t *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        if (e->from == from && e->to == to) return true;
    }

    return false;
}

graph_edge_t *graph_find_edge(graph_t *graph, graph_node_t *from, graph_node_t *to) {
    graph_edge_t *e;
    for (unsigned int i = vector_active(graph->edges); i--; /**/) {
        e = vector_slot(graph->edges, i);
        if (e->from == from && e->to == to) return e;
    }

    return NULL;
}
/*********************************** Views **************************************************/
char *graph_type_str(graph_t *graph) {
    return graph->type == Graph          ? "Graph"
           : graph->type == DiGraph      ? "DiGraph"
           : graph->type == MultiGraph   ? "MultiGraph"
           : graph->type == MultiDiGraph ? "MultiDiGraph"
                                         : "UnknownGraph";
}

void graph_dump_nodes(graph_t *graph) {
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

void graph_dump_edges(graph_t *graph) {
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

void graph_dump(graph_t *graph) {
    printf("%s = {\n", graph_type_str(graph));
    graph_dump_nodes(graph);
    graph_dump_edges(graph);
    printf("}\n");
}

int graph_number_of_nodes(graph_t *graph) { return graph->nodes->active; }

int graph_number_of_edges(graph_t *graph) { return graph->edges->active; }

graph_t *graph_reverse(graph_t *graph, bool copy) {
    if (graph->type == Graph || graph->type == MultiGraph) return graph;

    graph_node_t *node, *temp;
    graph_edge_t *edge;

    if (copy) {
        // TODO
    } else {
        for (unsigned int i = vector_active(graph->nodes); i--; /**/) {
            node = vector_slot(graph->nodes, i);
            temp = node->from;
            node->from = node->to;
            node->to = temp;
        }

        for (unsigned int i = vector_active(graph->edges); i--; /**/) {
            edge = vector_slot(graph->edges, i);
            temp = edge->from;
            edge->from = edge->to;
            edge->to = temp;
        }
    }

    return graph;
}

/*********************************** Generators **************************************************/

graph_t *graph_gen_trivial(graph_type_t t) {
    graph_t *g = graph_new(t);
    graph_add_node(g, 0, 0);
    return g;
}

graph_t *graph_gen_empty(int n, graph_type_t t) {
    int i;
    graph_t *g = graph_new(t);
    for (i = 0; i < n; i++) {
        graph_add_node(g, 0, 0);
    }
    return g;
}

graph_t *graph_gen_path(int n, graph_type_t t) {
    int i;
    graph_t *g = graph_new(t);
    for (i = 0; i < n; i++) graph_add_node(g, 0, 0);
    for (i = 0; i < g->nodes->active - 1; i++)
        graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, i + 1), 0, NULL);
    return g;
}

graph_t *graph_gen_cycle(int n, graph_type_t t) {
    graph_t *g = graph_gen_path(n, t);
    graph_add_edge(g, vector_slot(g->nodes, g->nodes->active - 1), vector_slot(g->nodes, 0), 0, NULL);
    return g;
}

graph_t *graph_gen_star(int n, graph_type_t t) {
    int i;
    graph_t *g = graph_new(t);
    graph_node_t *center, *node;
    center = graph_add_node(g, 0, 0);
    for (i = 0; i < n; i++) {
        node = graph_add_node(g, 0, 0);
        graph_add_edge(g, center, node, 0, NULL);
    }
    return g;
}

graph_t *graph_gen_wheel(int n, graph_type_t t) {
    if (n <= 0) return NULL;
    int i, j;
    graph_t *g = graph_gen_star(n - 1, t);
    for (i = 1; i < g->nodes->active; i++) {
        j = i == g->nodes->active - 1 ? 1 : i + 1;
        graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, j), 0, NULL);
    }
    return g;
}

graph_t *graph_gen_grid(int m, int n, graph_type_t t) {
    if (m == 1) return graph_gen_cycle(n, t);
    if (n == 1) return graph_gen_cycle(m, t);
    int i, j, k, u, d, l, r;
    graph_t *g = graph_gen_empty(m * n, t);

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
            if (u != -1) graph_add_edge(g, vector_slot(g->nodes, k), vector_slot(g->nodes, u), 0, NULL);
            if (r != -1) graph_add_edge(g, vector_slot(g->nodes, k), vector_slot(g->nodes, r), 0, NULL);
            if (d != -1) graph_add_edge(g, vector_slot(g->nodes, k), vector_slot(g->nodes, d), 0, NULL);
            if (l != -1) graph_add_edge(g, vector_slot(g->nodes, k), vector_slot(g->nodes, l), 0, NULL);
        }
    }
    return g;
}

graph_t *graph_gen_complete(int n, graph_type_t t) {
    int i, j;
    graph_t *g = graph_new(t);
    for (i = 0; i < n; i++) graph_add_node(g, 0, 0);

    for (i = 0; i < g->nodes->active; i++) {
        for (j = 0; j < g->nodes->active; j++) {
            if (i == j) continue;
            graph_add_edge(g, vector_slot(g->nodes, i), vector_slot(g->nodes, j), 0, NULL);
        }
    }
    return g;
}

graph_t *graph_gen_binomial_tree(int order, graph_type_t t) {}  // n = 2^order

graph_t *graph_gen_random(int n, graph_type_t t) {}
graph_t *graph_gen_random_tree(int n, graph_type_t t) {}

/*********************************** Algorithms **************************************************/
static void _graph_dfs(graph_t *graph, graph_node_t *start, vector visited, void (*dfs_cb)(graph_node_t *, void *),
                       void *arg) {
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
        graph_node_t *c = vector_slot(start->to, i);

        _graph_dfs(graph, c, visited, dfs_cb, arg);
    }
}

void graph_dfs(graph_t *graph, graph_node_t *start, void (*dfs_cb)(graph_node_t *, void *), void *arg) {
    vector visited = vector_init(VECTOR_MIN_SIZE);

    _graph_dfs(graph, start, visited, dfs_cb, arg);
    vector_free(visited);
}

void graph_dump_dot_default_print_cb(graph_node_t *gn) {
    char nbuf[64];

    for (unsigned int i = 0; i < vector_active(gn->to); i++) {
        graph_node_t *adj = vector_slot(gn->to, i);
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

// TODO
vector build_path_from_pred(int pred[]) {
    vector path = vector_init(VECTOR_MIN_SIZE);

    // vector_set(path, pred)
}

void graph_shortest_path(graph_t *graph, graph_node_t *from) {
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
    int pred[nn];

    // Initial all distances as INF and stpSet[] as false
    for (int i = 0; i < nn; i++) {
        dist[i] = INF;
        sptSet[i] = false;
        pred[i] = -1;
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
            graph_edge_t *e = graph_find_edge(graph, vector_slot(graph->nodes, u), vector_slot(graph->nodes, v));
            unsigned int weight = e ? (int)e->attr : 0;
            if (!sptSet[v] && e && dist[u] != INF && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pred[v] = u;
            }
        }
    }

    // print solution
    graph_node_t *n, *p;
    for (int i = 0; i < nn; i++) {
        if (pred[i] == -1) continue;
        n = vector_slot(graph->nodes, i);
        p = vector_slot(graph->nodes, pred[i]);
        printf("n(%d) -> n(%d): distance %d via n(%d)\n", from->id, n->id, dist[i], p->id);
    }
}

void _graph_shortest_path(graph_t *graph, graph_node_t *from, graph_node_t *to, edge_weight_type_t weight,
                          sp_algo_t method) {
    // if (method == Dijkstra) {
    // } else if (method == BellmanFord) {
    // } else {
    //     weight = Unweighted;
    //     printf("method not supported: {%s}", method);
    // }

    vector path = vector_init(VECTOR_MIN_SIZE);  // 0 to 4 is [0, 1, 2, 3, 4]

    if (from == NULL) {
        if (to == NULL) {
            // Find paths between all pairs.
            if (method == Dijkstra) {
                all_pairs_dijkstra_path(graph, weight);
            } else if (method == BellmanFord) {
                all_pairs_bellman_ford_path(graph, weight);
            } else {
                all_pairs_shortest_path(graph);
            }
        } else {
            // Find paths from all nodes co-accessible to the target.
            graph = graph_reverse(graph, false);
            if (method == Dijkstra) {
                single_source_dijkstra_path(graph, to, weight);
            } else if (method == BellmanFord) {
                single_source_bellman_ford_path(graph, to, weight);
            } else {
                single_source_shortest_path(graph, to);
            }
            // Now flip the paths so they go from a source to the target.
            // for target in paths:
            //     paths[target] = list(reversed(paths[target]))
        }
    } else {
        if (to == NULL) {
            // Find paths to all nodes accessible from the source.
            if (method == Dijkstra) {
                single_source_dijkstra_path(graph, from, weight);
            } else if (method == BellmanFord) {
                single_source_bellman_ford_path(graph, from, weight);
            } else {
                single_source_shortest_path(graph, from);
            }
        } else {
            //  Find shortest source-target path.
            if (method == Dijkstra) {
                bidirectional_dijkstra(graph, from, to, weight);
            } else if (method == BellmanFord) {
                bellman_ford_path(graph, from, to, weight);
            } else {
                bidirectional_shortest_path(graph, from, to);
            }
        }
    }
}

vector all_pairs_dijkstra_path(graph_t *graph, edge_weight_type_t weight) {}
vector all_pairs_bellman_ford_path(graph_t *graph, edge_weight_type_t weight) {}
vector all_pairs_shortest_path(graph_t *graph) {}
vector single_source_dijkstra_path(graph_t *graph, graph_node_t *from, edge_weight_type_t weight) {}
vector single_source_bellman_ford_path(graph_t *graph, graph_node_t *from, edge_weight_type_t weight) {}
vector single_source_shortest_path(graph_t *graph, graph_node_t *from) {}
vector bidirectional_dijkstra(graph_t *graph, graph_node_t *from, graph_node_t *to, edge_weight_type_t weight) {}
vector bellman_ford_path(graph_t *graph, graph_node_t *from, graph_node_t *to, edge_weight_type_t weight) {}
vector bidirectional_shortest_path(graph_t *graph, graph_node_t *from, graph_node_t *to) {}
// void graph_dump_dot_default_print_cb(graph_node_t *gn, struct buffer *buf)
// {
// 	char nbuf[64];

// 	for (unsigned int i = 0; i < vector_active(gn->to); i++) {
// 		graph_node_t *adj = vector_slot(gn->to, i);

// 		snprintf(nbuf, sizeof(nbuf), "    n%p -> n%p;\n", gn, adj);
// 		buffer_putstr(buf, nbuf);
// 	}
// }

// char *graph_dump_dot(graph_t *graph, graph_node_t *start,
// 		     void (*pcb)(graph_node_t *, struct buffer *))
// {
// 	struct buffer *buf = buffer_new(0);
// 	char *ret;

// 	pcb = (pcb) ? pcb : graph_dump_dot_default_print_cb;
// 	buffer_putstr(buf, "digraph {\n");

// 	graph_dfs(graph, start, (void (*)(graph_node_t *, void *))pcb,
// 		  buf);

// 	buffer_putstr(buf, "}\n");

// 	ret = buffer_getstr(buf);
// 	buffer_free(buf);

// 	return ret;
// }