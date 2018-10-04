// graph_layout_algorithms.h
#pragma once

#include "view.h"

#include <vector>

/*
 * dfs_grid_layout takes a view and returns a view with node positions
 * determined.
 * It creates a DFS tree and assigns positions in a grid-like manner
 * It is expected that the node_extents has already been filled in for the
 * algorithm to access
 */
void dfs_grid_layout(View &);
void expand_node(View &view, NodeBase *node);
void collapse_node(View &view, NodeBase *node);
std::vector<NodeBase*> get_nodes_to_collapse(const View&, NodeBase*);
