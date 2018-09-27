// view_filters.h
#pragma once

#include "geometry.h"
#include "view.h"

#include <algorithm>
#include <vector>

LineSegment PhysicalEdge(const View &view, const EdgeBase &edge);
std::vector<LineSegment> PhysicalNeighborhood(const View &view,
                                              const NodeBase &node);

void prune_isolated_nodes(View &view);

void set_logicalView(View &view, const std::vector<NodeBase *>& nodes);
void set_physicalView(View &view, const Rectangle &view_box);
