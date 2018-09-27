// main_functions.h
#pragma once

#include "drawingarea_zoom_drag.h" //included for type aliases
#include "graph.h"
#include "graph_layout_algorithms.h"
#include "view.h"
#include "view_filters.h"

#include <gtkmm-3.0/gtkmm.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <regex>
#include <string>

struct NodeClickInfo {
  Node *node;
  GdkEventButton *e;

  void set(Node *node_, GdkEventButton *e_);
  void clear();

  NodeClickInfo &operator=(NodeClickInfo &&nodeClickInfo);
  NodeClickInfo(NodeClickInfo &&nodeClickInfo);
  NodeClickInfo();
  NodeClickInfo &operator=(const NodeClickInfo&) = default;
  ~NodeClickInfo();

  explicit operator bool() { return node && e; }
};

struct MyState {
  NodeClickInfo nodeClick;
  NodeClickInfo node2Click;
  bool handle_event_click(Node*, GdkEventButton *e);
  DragTarget get_motion_target() const;
};

int usage();

std::string remove_qualifiers(const Fullname &fullname);

void initialize_view(const Graph &graph, View &view, PLayout &layout);

void draw_node(const View &view, const NodeBase *node, CContext c,
               PLayout layout);
void draw_edge(const View &view, const EdgeBase *edge, CContext c);
bool draw_view(const View &view, CContext c, PLayout layout);

Node *find_node(const View &view, const Point &point);

Extent get_extent(const Gtk::Widget &widget);

/*
 * If the view box is not contained by the view box determined by the
 * physicalSubView.box, then a new one is calculated.  This is to prevent
 * calculating a new box for every tiny little change to the view.
 */
bool check_physicalSubView(View &view, Rectangle view_box);