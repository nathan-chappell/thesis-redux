// main_functions.cc

#include "main_functions.h"
#include <vector>

using namespace std;

const string qualified_re = "(?:\\w*::)+";
const double PI = 4 * atan(1);

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

NodeClickInfo &NodeClickInfo::operator=(NodeClickInfo &&nodeClickInfo) {
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

bool MyState::handle_event_click(Node *node, GdkEventButton *e) {
  if (!node) {
    nodeClick.clear();
    node2Click.clear();
    return false;
  }
  switch (e->type) {
  case GDK_2BUTTON_PRESS: {
    DIAGNOSTIC << "2clicked: " << *node << endl;
    node2Click = move(nodeClick);
    /*
     * handle animation sequence, get a connection for myState.viewAnimation
     */
  } break;
  case GDK_BUTTON_PRESS: {
    DIAGNOSTIC << "clicked: " << *node << endl;
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
    view.viewData[node].expanded = false;
    DIAGNOSTIC << "initialized: " << text << " : " << node << " : "
               << *view.viewData[node].box << endl;
  }
  view.roots = (move(graph.get_roots()));
  set_logicalView(view, view.roots);
}

void draw_node_rectangle(const View &view, NodeBase *node, CContext c,
                         PLayout layout) {
  const Point &point = view.viewData.at(node).box->position;
  const Extent &extent = view.viewData.at(node).box->extent;
  if (view.viewData.at(node).expanded) {
    c->set_line_width(3 * c->get_line_width());
  }
  // rectangle
  c->rectangle(point.x, point.y, extent.x, extent.y);
  c->stroke();
  // text
  c->move_to(point.x + view.node_margin / 2.0,
             point.y + view.node_margin / 2.0);
  layout->set_text(view.viewData.at(node).text);
  layout->show_in_cairo_context(c);
}

void draw_node_in_circle(const View &view, NodeBase *node, CContext c,
                         PLayout layout) {
  // circles for incoming
  if (node->out_degree()) {
    Point right_m = view.viewData.at(node).box->Right().MidPoint();
    c->set_source_rgb(0, 1, 0);
    c->arc(right_m.x, right_m.y, 5, 0, 2 * PI);
    c->fill();
    c->stroke();
  }
}

void draw_node_out_circle(const View &view, NodeBase *node, CContext c,
                          PLayout layout) {
  // circles for outgoing
  if (node->in_degree()) {
    Point right_m = view.viewData.at(node).box->Left().MidPoint();
    c->set_source_rgb(1, 0, 0);
    c->arc(right_m.x, right_m.y, 5, 0, 2 * PI);
    c->fill();
    c->stroke();
  }
}

typedef void (*draw_function)(const View &view, NodeBase *node, CContext c,
                              PLayout layout);

const vector<draw_function> draw_functions = {
    draw_node_rectangle, draw_node_in_circle, draw_node_out_circle};

void draw_node(const View &view, const NodeBase *cnode, CContext c,
               PLayout layout) {
  NodeBase *node = const_cast<NodeBase *>(cnode);
  for (auto fn : draw_functions) {
    c->save();
    fn(view, node, c, layout);
    c->restore();
  }
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
  if (!view.physicalSubView.force_recalculate &&
      view.physicalSubView.box.Contains(view_box)) {
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

// t \in [0,1]
Point linear_animate(const Point &start, const Point &end, double cur_t,
                     double max_t) {
  return ((max_t - cur_t) * start + (cur_t)*end) / max_t;
}

bool ViewAnimation::is_finished() const {
  // if (time_period > 0) return time >= final_time;
  // return time_period > 0 ? time >= final_time : time <= 0;
  return time >= final_time;
}

bool ViewAnimation::is_valid() const { return final_time && time_period; }

ViewAnimation::operator bool() const { return is_valid() && !is_finished(); }

ViewAnimation &ViewAnimation::operator++() {
  if (!is_valid()) {
    DIAGNOSTIC << " ViewAnimation no longer valid" << endl;
    return *this;
  }
  for (auto node :
       current_view.physicalSubView.nodes) { // current vs final may be problem
    current_view.viewData.at(node).box->position = linear_animate(
        initial_view.viewData.at(node).box->position,
        final_view.viewData.at(node).box->position, time, final_time);
  }
  time += time_period;
  return *this;
};

void ViewAnimation::init(Gtk::DrawingArea &da, View &view,
                         ViewTransform transform, ViewTransform cleanup) {
  DIAGNOSTIC << "initing animation" << endl;
  view.physicalSubView.force_recalculate = true;
  initial_view = move(view);
  current_view = initial_view;
  current_view.detach_node_boxes();
  final_view = initial_view;
  final_view.detach_node_boxes();
  transform(final_view);
  set_default_timing();
  Glib::signal_timeout().connect(
      [&,cleanup,this]() {
        da.queue_draw();
        if (is_finished()) {
          DIAGNOSTIC << "cleaning up" << endl;
          cleanup(final_view);
          DIAGNOSTIC << "moving" << endl;
          view = move(final_view);
          invalidate();
        }
        return !is_finished();
      },
      time_period);
}

void ViewAnimation::set_default_timing() {
  time = 0;
  final_time = 1000;
  time_period = 20;
}

void ViewAnimation::invalidate() {
  final_time = 0;
  time_period = 0;
}
