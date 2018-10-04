// view.h
#pragma once

// header only file!

#include "geometry.h"
#include "node_base.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct NodeViewData;

using ViewData = std::unordered_map<NodeBase *, NodeViewData>;
using LogicalSubView = std::unordered_set<NodeBase *>;

// SEE initialize_view
struct NodeViewData {
  std::shared_ptr<Rectangle> box;
  std::string text;
  bool expanded;
};

struct PhysicalSubView {
  LogicalSubView nodes;
  Rectangle box;
  bool force_recalculate;
};

struct View {
  ViewData viewData;
  /*
   * This is a "weak link" to the graph object, perhaps a shared_ptr would be
   * better...
   */
  std::unordered_map<Fullname, NodeBase *> *name_to_node;
  std::unordered_set<NodeBase *> logicalSubView;
  std::vector<NodeBase *> roots;
  PhysicalSubView physicalSubView;
  double node_margin;
  double row_spacing;
  double column_spacing;

  View() : name_to_node{nullptr} {}
  View(std::unordered_map<Fullname, NodeBase *> *name_to_node)
      : name_to_node(name_to_node), node_margin{10.0}, row_spacing{30.0},
        column_spacing{30.0} {}

  /*
   * Use this after copying a view to make the node boxes independent of one
   * another.  Useful for animations.
   */
  void detach_node_boxes() {
    for (auto node : logicalSubView) {
      viewData.at(node).box =
          std::make_shared<Rectangle>(*viewData.at(node).box);
    }
  }
};
