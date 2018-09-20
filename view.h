// view.h
#pragma once

// header only file!

#include "node_base.h"

#include <complex>
#include <memory>
#include <string>
#include <unordered_map>

struct NodeViewData;

using C = std::complex<double>;
using ViewData = std::unordered_map<NodeBase *, NodeViewData>;

struct NodeViewData {
  std::shared_ptr<C> position;
  C node_extents; // provide height/width of a bounding box.
  std::string text;
  // add other attributes later... color, is_hidden, et cetera

  //NodeViewData() : position{std::make_shared<C>()} {}
};

struct View {
  ViewData viewData;
  /*
   * This is a "weak link" to the graph object, perhaps a shared_ptr would be better...
   */
  std::unordered_map<Fullname, NodeBase *>
      *name_to_node;
  double node_margin;
  double row_spacing;
  double column_spacing;

  View(std::unordered_map<Fullname, NodeBase *> *name_to_node)
      : name_to_node(name_to_node), node_margin{10.0}, row_spacing{30.0},
        column_spacing{30.0} {}
};
