// main.cc

#include "drawingarea_zoom_drag.h"
#include "graph.h"
#include "graph_layout_algorithms.h"
#include "view.h"
#include "view_filters.h"

#include <gtkmm-3.0/gtkmm.h>

#include <algorithm>
#include <functional>
#include <iostream>

using PLayout = Glib::RefPtr<Pango::Layout>;

using namespace std;

string get_unqualified_name(const Fullname &fullname) {
  /*
  size_t last_colon = fullname.find_last_of(':');
  return last_colon == string::npos ? fullname
                                    : fullname.substr(last_colon + 1);
                                    */
  return fullname;
}

void initialize_view(const Graph &graph, View &view, PLayout &layout) {

  for (auto &&node : graph.nodes) {
    string text = get_unqualified_name(node->fullname);
    view.viewData[node].text = text;
    layout->set_text(text);
    Pango::Rectangle r = layout->get_pixel_ink_extents();
    view.viewData[node].node_extents = C(2 * view.node_margin + r.get_width(),
                                         2 * view.node_margin + r.get_height());
    view.viewData[node].position = make_shared<C>();
  }
}

void draw_node(const NodeViewData &node, CContext c, PLayout layout,
               const double &margin) {
  c->rectangle(node.position->real(), node.position->imag(),
               node.node_extents.real(), node.node_extents.imag());
  c->stroke();
  c->move_to(node.position->real() + margin / 2.0,
             node.position->imag() + margin / 2.0);
  layout->set_text(node.text);
  layout->show_in_cairo_context(c);
}

C get_right_middle(const NodeViewData &nodeViewData) {
  return *nodeViewData.position + C(nodeViewData.node_extents.real(),
                                    nodeViewData.node_extents.imag() / 2.0);
}

C get_left_middle(const NodeViewData &nodeViewData) {
  return *nodeViewData.position +
         C(0.0, nodeViewData.node_extents.imag() / 2.0);
}

void draw_edge(const NodeViewData &tail, const NodeViewData &head, CContext c) {
  C from = get_right_middle(tail);
  C to = get_left_middle(head);
  c->move_to(from.real(), from.imag());
  c->line_to(to.real(), to.imag());
  c->stroke();
}

bool draw_view(View &view, CContext c, PLayout layout) {
  for (auto &&kv : view.viewData) {
    draw_node(kv.second, c, layout, view.node_margin);
  }
  for (auto &&kv : view.viewData) {
    for (auto edge : kv.first->neighborhood.outgoing) {
      auto tail_kv_it = view.viewData.find(edge->tail);
      auto head_kv_it = view.viewData.find(edge->head);
      if (tail_kv_it == view.viewData.end() ||
          head_kv_it == view.viewData.end()) {
        cout << "failed to draw edge" << *edge << endl;
      }
      draw_edge(tail_kv_it->second, head_kv_it->second, c);
    }
  }
  return false;
}

bool contains_point(const NodeViewData &nodeViewData, double x, double y,
                    const DrawingArea_ZoomDrag &drawingArea_ZoomDrag) {
  C ul = *nodeViewData.position;
  C lr = *nodeViewData.position + nodeViewData.node_extents;
  C click = drawingArea_ZoomDrag.user_to_image(C(x, y));
  // remember, y increases as you go down the screen
  return ul.real() <= click.real() && click.real() <= lr.real() &&
         ul.imag() <= click.imag() && click.imag() <= lr.imag();
}

int usage() {
  cout << "usage: ./graph <filename>" << endl;
  cout << "  The filename should indicate a file created with get_call_graph"
       << endl;
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return usage();
  }
  Graph graph = parseCallGraphFromFile(argv[1]);
  dump_call_graph(graph);

  View view(&graph.name_to_node);

  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create();
  Gtk::Window window;
  DrawingArea_ZoomDrag drawingArea_ZoomDrag;
  window.add(drawingArea_ZoomDrag);
  PLayout layout =
      Pango::Layout::create(drawingArea_ZoomDrag.get_pango_context());

  initialize_view(graph, view, layout);
  prune_isolated_nodes(view);
  get_initialViewFullGraph(view);
  drawingArea_ZoomDrag.zoomed_draw = [&view, layout](CContext c) {
    return draw_view(view, c, layout);
  };

  drawingArea_ZoomDrag.set_drag =
      [&view, &drawingArea_ZoomDrag](GdkEventButton *e) -> shared_ptr<C> {
    auto kv_it = find_if(
        view.viewData.begin(), view.viewData.end(),
        [e, &drawingArea_ZoomDrag](const ViewData::value_type &kv) {
          return contains_point(kv.second, e->x, e->y, drawingArea_ZoomDrag);
        });
    if (kv_it != view.viewData.end()) {
      return kv_it->second.position;
    }
    return nullptr;
  };

  window.set_default_size(800, 800);
  window.show_all();

  app->run(window);
}
