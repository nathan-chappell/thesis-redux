// graph_layout_algorithms.h
#pragma once

#include "view.h"

/*
 * get_initialViewFullGraph takes a view and returns a view with node positions
 * determined.
 * It creates a DFS tree and assigns positions in a grid-like manner
 * It is expected that the node_extents has already been filled in for the
 * algorithm to access
 */
void get_initialViewFullGraph(View &);
