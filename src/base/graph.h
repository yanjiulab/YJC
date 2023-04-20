#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

enum graph_type { Graph, DiGraph };

struct graph_node {
    vector from;  // nodes which have edges to this node
    vector to;    // nodes which this node has edges to

    unsigned int id;          // node id
    void *data;               // node data
    void (*del)(void *data);  // deletion callback
};
typedef struct graph_node graph_node_t;

struct graph_edge {
    struct graph_node *from;
    struct graph_node *to;

    void *attr;               // edge attr
    void (*del)(void *addr);  // deletion callback
};
typedef struct graph_edge graph_edge_t;

struct graph {
    int max_id;
    vector nodes;  // all nodes
    vector edges;  // all edges
};
typedef struct graph graph_t;

/**
 * Create a new graph
 *
 * @return the new graph pointer
 */
struct graph *graph_new(void);

/**
 * Deletes a graph.
 * Calls graph_delete_node on each node before freeing the graph struct itself.
 *
 * @param graph the graph to delete
 */
void graph_delete(struct graph *graph);

/**
 * Dump a graph.
 *
 * @param graph the graph to dump
 */
void graph_dump_nodes(struct graph *graph);
void graph_dump_edges(struct graph *graph);
void graph_dump(struct graph *graph);
int graph_number_of_nodes(struct graph *graph);
int graph_number_of_edges(struct graph *graph);

/**
 * Creates a new node and add to graph
 *
 * @struct graph the graph this node exists in
 * @param[in] data this node's data
 * @param[in] del data deletion callback
 * @return the new node
 */
struct graph_node *graph_add_node(struct graph *graph, void *data, void (*del)(void *));

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
void graph_delete_node(struct graph *graph, struct graph_node *node);

/**
 * Makes a directed edge between two nodes.
 *
 * @param[in] from
 * @param[in] to
 * @return to
 */
struct graph_edge *graph_add_edge(struct graph *graph, struct graph_node *from, struct graph_node *to, void *attr,
                                  void (*del)(void *));

/**
 * Removes a directed edge between two nodes.
 *
 * @param[in] from
 * @param[in] to
 */
void graph_delete_edge(struct graph *graph, struct graph_node *from, struct graph_node *to);

/*
 * Finds a node in the graph.
 *
 * @param[in] graph the graph to search in
 * @param[in] data the data to key off
 * @return the first graph node whose data pointer matches `data`
 */
struct graph_node *graph_find_node(struct graph *graph, void *data);
struct graph_edge *graph_find_edge(struct graph *graph, struct graph_node *from, struct graph_node *to);
/*
 * Determines whether two nodes have a directed edge between them.
 *
 * @param from
 * @param to
 * @return whether there is a directed edge from `from` to `to`.
 */
bool graph_has_edge(struct graph *graph, struct graph_node *from, struct graph_node *to);

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
void graph_dfs(struct graph *graph, struct graph_node *start, void (*dfs_cb)(struct graph_node *, void *), void *arg);

void graph_shortest_path(struct graph *graph, struct graph_node *from);

/*********************************** Generators **************************************************/

/**
 * @brief Return the Trivial graph with one node (with label 0) and no edges.
 * 
 * @return struct graph* 
 */
struct graph *graph_gen_trivial();

/**
 * @brief Returns the empty graph with n nodes and zero edges.
 *
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_empty(int n);

/**
 * @brief Returns the Path graph of linearly connected nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_path(int n);

/**
 * @brief Returns the cycle graph of cyclically connected nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_cycle(int n);

/**
 * @brief The star graph consists of one center node connected to n outer nodes.
 *
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_star(int n);


/**
 * @brief Return the wheel graph
 * The wheel graph consists of a hub node connected to a cycle of (n-1) nodes.
 * 
 * @param n 
 * @return struct graph* 
 */
struct graph *graph_gen_wheel(int n);


/**
 * @brief The grid graph has each node connected to its four nearest neighbors.
 *
 * @param m
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_grid(int m, int n);

/**
 * @brief Return the complete graph K_n with n nodes.
 * A complete graph on n nodes means that all pairs of distinct nodes have an edge connecting them.
 *
 * @param n
 * @return struct graph*
 */
struct graph *graph_gen_complete(int n);

struct graph *graph_gen_random(int n);
struct graph *graph_gen_random_tree(int n);
struct graph *graph_gen_binomial_tree(int order);

#ifdef __cplusplus
}
#endif

#endif