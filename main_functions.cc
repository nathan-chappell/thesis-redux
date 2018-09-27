// main_functions.cc

#include "main_functions.h"

using namespace std;

const string qualified_re = "(?:\\w*::)+";

string remove_qualifiers(const Fullname &fullname) {
  return regex_replace(fullname, regex(qualified_re), "");
}

void NodeClickInfo::set(Node *node_, GdkEventButton *e_) {
  node = node_;
  e = e_;
}

void NodeClickInfo::clear() {
  node = nullptr;
  e = nullptr;
}

NodeClickInfo& NodeClickInfo::operator=(NodeClickInfo &&nodeClickInfo) {
  node = nodeClickInfo.node;
  e = nodeClickInfo.e;
  nodeClickInfo.clear();
  return *this;
}

NodeClickInfo::NodeClickInfo(NodeClickInfo &&nodeClickInfo) {
  *this = move(nodeClickInfo);
}
NodeClickInfo::NodeClickInfo() : node(nullptr), e(nullptr) {}
NodeClickInfo::~NodeClickInfo() { clear(); }

bool MyState::handle_event_click(Node* node, GdkEventButton *e) {
  if (!node) {
    nodeClick.clear();
    node2Click.clear();
    return false;
  }
  switch (e->type) {
  case GDK_2BUTTON_PRESS: {
    cout << "2clicked: " << *node << endl;
    node2Click = move(nodeClick);
  } break;
  case GDK_BUTTON_PRESS: {
    cout << "clicked: " << *node << endl;
    nodeClick.set(node, e);
  } break;
  default: {
    // ummm
  }
  }
  return false;
}

void initialize_view(const Graph &graph, View &view, PLayout &layout) {
  for (auto node : graph.nodes) {
    string text = remove_qualifiers(node->fullname);
    view.viewData[node].text = text;
    layout->set_text(text);
    Pango::Rectangle r = layout->get_pixel_ink_extents();
    Extent extent(2 * view.node_margin + r.get_width(),
                  2 * view.node_margin + r.get_height());
    view.viewData[node].box = make_shared<Rectangle>(Point(), extent);
  }
  view.roots = (move(graph.get_roots()));
  set_logicalView(view, view.roots);
}

void draw_node(const View &view, const NodeBase *node, CContext c,
               PLayout layout) {
  const Point &point =
      view.viewData.at(const_cast<NodeBase *>(node)).box->position;
  const Extent &extent =
      view.viewData.at(const_cast<NodeBase *>(node)).box->extent;
  c->rectangle(point.x, point.y, extent.x, extent.y);
  c->stroke();
  c->move_to(point.x + view.node_margin / 2.0,
             point.y + view.node_margin / 2.0);
  layout->set_text(view.viewData.at(const_cast<NodeBase *>(node)).text);
  layout->show_in_cairo_context(c);
}

void draw_edge(const View &view, const EdgeBase *edge, CContext c) {
  LineSegment physicalEdge = PhysicalEdge(view, *edge);
  c->move_to(physicalEdge.u.x, physicalEdge.u.y);
  c->line_to(physicalEdge.v.x, physicalEdge.v.y);
  c->stroke();
}

bool draw_view(const View &view, CContext c, PLayout layout) {
  unordered_set<EdgeBase *> drawn_edges;
  for (auto &&node : view.physicalSubView.nodes) {
    draw_node(view, node, c, layout);
    // the nodes of the physical subview are calculated based on whether or not
    // their edges intersect the view, so this should work...
    for (auto edge : node->neighborhood.outgoing) {
      if (view.logicalSubView.count(edge->tail) &&
          view.logicalSubView.count(edge->head) && !drawn_edges.count(edge)) {
        draw_edge(view, edge, c);
        drawn_edges.insert(edge);
      }
    }
  }
  return false;
}

int usage() {
  cout << "usage: ./graph <filename>" << endl;
  cout << "  The filename should indicate a file created with get_call_graph"
       << endl;
  return 1;
}

Extent get_extent(const Gtk::Widget &widget) {
  return Extent(widget.get_width(), widget.get_height());
}

bool check_physicalSubView(View &view, Rectangle view_box) {
  if (view.physicalSubView.box.Contains(view_box)) {
    return false;
  }
  // make the "view box" slightly larger
  view_box.position -= view_box.extent / 2.0;
  view_box.extent *= 2;
  set_physicalView(view, view_box);
  return true;
}

Node *find_node(const View &view, const Point &point) {
  auto result = find_if(view.physicalSubView.nodes.begin(),
                        view.physicalSubView.nodes.end(),
                        [&point, &view](NodeBase *node) {
                          return view.viewData.at(node).box->Contains(point);
                        });
  return result == view.physicalSubView.nodes.end()
             ? nullptr
             : dynamic_cast<Node *>(*result);
}
