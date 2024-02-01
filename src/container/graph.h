#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum graph_type { Graph,
                          DiGraph,
                          MultiGraph,
                          MultiDiGraph } graph_type_t;
typedef enum edge_weight_type { Unweighted,
                                Int,
                                Double } edge_weight_type_t;
typedef enum sp_algo { Dijkstra,
                       BellmanFord } sp_algo_t;

typedef struct graph_node {
    vector from; // nodes which have edges to this node
    vector to;   // nodes which this node has edges to

    unsigned int id;         // node id
    void* data;              // node data
    void (*del)(void* data); // deletion callback
} graph_node_t;

typedef struct graph_edge {
    struct graph_node* from;
    struct graph_node* to;

    void* attr;              // edge attr
    void (*del)(void* addr); // deletion callback
} graph_edge_t;

typedef struct graph {
    int max_id; // record
    graph_type_t type;
    vector nodes; // all nodes
    vector edges; // all edges
} graph_t;

/*********************************** Basic **************************************************/
/**
 * Create a new graph
 *
 * @return the new graph pointer
 */
struct graph* graph_new(graph_type_t t);

/**
 * Deletes a graph.
 * Calls graph_delete_node on each node before freeing the graph struct itself.
 *
 * @param graph the graph to delete
 */
void graph_delete(struct graph* graph);

/**
 * Creates a new node and add to graph
 *
 * @struct graph the graph this node exists in
 * @param[in] data this node's data
 * @param[in] del data deletion callback
 * @return the new node
 */
struct graph_node* graph_add_node(struct graph* graph, void* data, void (*del)(void*));

/**
 * Deletes a node.
 *
 * Before deletion, this function removes all edges to and from this node from
 * any neighbor nodes.
 *
 * If *data and *del are non-null, the following call is made:
 *   (*node->del) (node->data);
 *
 * @param[in] graph the graph this node belongs to
 * @param[out] node pointer to node to delete
 */
void graph_delete_node(struct graph* graph, struct graph_node* node);

/**
 * Makes a directed edge between two nodes.
 *
 * @param[in] from
 * @param[in] to
 * @return to
 */
struct graph_edge* graph_add_edge(struct graph* graph, struct graph_node* from, struct graph_node* to, void* attr,
                                  void (*del)(void*));

/**
 * Removes a directed edge between two nodes.
 *
 * @param[in] from
 * @param[in] to
 */
void graph_delete_edge(struct graph* graph, struct graph_node* from, struct graph_node* to);

/*
 * Finds a node in the graph.
 *
 * @param[in] graph the graph to search in
 * @param[in] data the data to key off
 * @return the first graph node whose data pointer matches `data`
 */
struct graph_node* graph_find_node(struct graph* graph, void* data);

/**
 * Finds an edge in the graph.
 *
 * @return struct graph_edge*
 */
struct graph_edge* graph_find_edge(struct graph* graph, struct graph_node* from, struct graph_node* to);

/*
 * Determines whether two nodes have a directed edge between them.
 *
 * @param from
 * @param to
 * @return whether there is a directed edge from `from` to `to`.
 */
bool graph_has_edge(struct graph* graph, struct graph_node* from, struct graph_node* to);

/*********************************** Views **************************************************/
/**
 * Dump a graph.
 *
 * @param graph the graph to dump
 */
void graph_dump_nodes(struct graph* graph);
void graph_dump_edges(struct graph* graph);
void graph_dump(struct graph* graph);
int graph_number_of_nodes(struct graph* graph);
int graph_number_of_edges(struct graph* graph);
graph_t* graph_reverse(graph_t* graph, bool copy);

/*********************************** Generators **************************************************/

/**
 * @brief Return the Trivial graph with one node (with label 0) and no edges.
 *
 * @return struct graph*
 */
struct graph* graph_gen_trivial(graph_type_t t);

/**
 * @brief Returns the empty graph with n nodes and zero edges.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_empty(int n, graph_type_t t);

/**
 * @brief Returns the Path graph of linearly connected nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_path(int n, graph_type_t t);

/**
 * @brief Returns the cycle graph of cyclically connected nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_cycle(int n, graph_type_t t);

/**
 * @brief The star graph consists of one center node connected to n outer nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_star(int n, graph_type_t t);

/**
 * @brief Return the wheel graph
 * The wheel graph consists of a hub node connected to a cycle of (n-1) nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_wheel(int n, graph_type_t t);

/**
 * @brief The grid graph has each node connected to its four nearest neighbors.
 *
 * @param m
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_grid(int m, int n, graph_type_t t);

/**
 * @brief Return the complete graph K_n with n nodes.
 * A complete graph on n nodes means that all pairs of distinct nodes have an edge connecting them.
 *
 * @param n
 * @return struct graph*
 */
struct graph* graph_gen_complete(int n, graph_type_t t);

struct graph* graph_gen_random(int n, graph_type_t t);
struct graph* graph_gen_random_tree(int n, graph_type_t t);
struct graph* graph_gen_binomial_tree(int order, graph_type_t t);

/*********************************** Algorithms **************************************************/

/*
 * Depth-first search.
 *
 * Performs a depth-first traversal of the given graph, visiting each node
 * exactly once and calling the user-provided callback for each visit.
 *
 * @param graph the graph to operate on
 * @param start the node to take as the root
 * @param dfs_cb callback called for each node visited in the traversal
 * @param arg argument to provide to dfs_cb
 */
void graph_dfs(struct graph* graph, struct graph_node* start, void (*dfs_cb)(struct graph_node*, void*), void* arg);

void graph_shortest_path(struct graph* graph, struct graph_node* from);

vector all_pairs_dijkstra_path(graph_t* graph, edge_weight_type_t weight);
vector all_pairs_bellman_ford_path(graph_t* graph, edge_weight_type_t weight);
vector all_pairs_shortest_path(graph_t* graph);
vector single_source_dijkstra_path(graph_t* graph, graph_node_t* from, edge_weight_type_t weight);
vector single_source_bellman_ford_path(graph_t* graph, graph_node_t* from, edge_weight_type_t weight);
vector single_source_shortest_path(graph_t* graph, graph_node_t* from);
vector bidirectional_dijkstra(graph_t* graph, graph_node_t* from, graph_node_t* to, edge_weight_type_t weight);
vector bellman_ford_path(graph_t* graph, graph_node_t* from, graph_node_t* to, edge_weight_type_t weight);
vector bidirectional_shortest_path(graph_t* graph, graph_node_t* from, graph_node_t* to);

#ifdef __cplusplus
}
#endif

#endif