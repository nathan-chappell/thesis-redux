// main.cc

#include "drawingarea_zoom_drag.h"
#include "geometry.h"
#include "graph.h"
#include "graph_layout_algorithms.h"
#include "main_functions.h"
#include "view.h"
#include "view_filters.h"

#include <gtkmm-3.0/gtkmm.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <regex>
#include <unordered_set>

using namespace std;

Rectangle get_view_box(const DrawingArea_ZoomDrag &drawingArea_ZoomDrag) {
  Rectangle view_box(Point(0, 0),
                     Point(move(get_extent(drawingArea_ZoomDrag))));
  drawingArea_ZoomDrag.user_to_image(view_box);
  return view_box;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return usage();
  }
  /*
   * Getting the underlying graph.
   * TODO Persistance and multiple views (view views in a view tree)
   */
  Graph graph = parseCallGraphFromFile(argv[1]);
  dump_call_graph(graph, cerr); // Useful for debug type of stuff...

  /*
   * The view holds the information necessary to display the graph as the user
   * desires.
   * TODO Make the attributes of the View interactive (i.e. add node expansion
   * and filtering)
   * TODO Keyboard navigation of the graph (hjkl)
   * TODO Multiple node selection
   * TODO Hover causes the qualified name to appear
   * TODO Double Click shows the source code
   * TODO Right Click context menu (lock position, change color/ shape)
   * TODO Different layout algorithms (center nodes on children, treemap,
   * nondeterministic cluster)
   * TODO Relevant edge info => implies parsing more interesting graphs =>
   * building more useful interface to clang
   */
  View view(&graph.name_to_node);

  MyState myState;
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create();
  Gtk::Window window;
  DrawingArea_ZoomDrag drawingArea_ZoomDrag;
  window.add(drawingArea_ZoomDrag);
  PLayout layout =
      Pango::Layout::create(drawingArea_ZoomDrag.get_pango_context());

  initialize_view(graph, view, layout);
  prune_isolated_nodes(view);
  dfs_grid_layout(view);

  drawingArea_ZoomDrag.zoomed_draw = [&view, &layout, &myState,
                                      &drawingArea_ZoomDrag](CContext c) {
    Rectangle view_box = get_view_box(drawingArea_ZoomDrag);
    View *rview = &view;
    if (++myState.viewAnimation) {
      rview = &myState.viewAnimation.current_view;
    }
    // check that our physical view covers the drawing area
    check_physicalSubView(*rview, view_box);
    return draw_view(*rview, c, layout);
  };

  drawingArea_ZoomDrag.signal_button_press_event().connect(
      [&](GdkEventButton *e) {
        // DIAGNOSTIC << "button_press lambda" << endl;
        Point click(e->x, e->y);
        drawingArea_ZoomDrag.user_to_image(click);
        Node *node = find_node(view, click);
        myState.handle_event_click(node, e);
        if (myState.node2Click && e->type == GDK_2BUTTON_PRESS) {
          if (!view.viewData.at(myState.node2Click.node).expanded) {
            // expand the node
            expand_node(view, node);
            DIAGNOSTIC << "expanding node animation: " << node << endl;
            myState.viewAnimation.init(drawingArea_ZoomDrag, view,
                                       dfs_grid_layout, [](View &v) {});
          } else {
            // collapse the node
            auto nodes_to_collapse = get_nodes_to_collapse(view, node);
            myState.viewAnimation.init(
                drawingArea_ZoomDrag, view,
                [&,node](View &lview) { collapse_node(lview, node); },
                [nodes_to_collapse](View &lview) {
                  for (auto lnode : nodes_to_collapse) {
                  cout << "must erase node: " << lnode << endl;
                    lview.logicalSubView.erase(lnode);
                    lview.physicalSubView.nodes.erase(lnode);
                  }
                });
          }
        }
        return false;
      },
      false);

  drawingArea_ZoomDrag.signal_motion_notify_event().connect(
      [&](GdkEventMotion *e) {
        // DIAGNOSTIC << "motion lambda" << endl;
        if (myState.nodeClick) {
          drawingArea_ZoomDrag.set_dragTarget(
              view.viewData[myState.nodeClick.node].box);
        }
        return false;
      },
      false);

  window.set_default_size(800, 800);
  window.show_all();

  app->run(window);
}
