#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif