// view_filters.cc

#include "myassert.h"
#include "view_filters.h"

using namespace std;

LineSegment PhysicalEdge(const View &view, const EdgeBase &edge) {
  try {
    return LineSegment(view.viewData.at(edge.tail).box->Right().MidPoint(),
                       view.viewData.at(edge.head).box->Left().MidPoint());
  } catch (out_of_range &e) {
    cerr << "couldn't find: " << edge.tail << " -> " << edge.head << endl;
    throw e;
  }
  return LineSegment();
}

string node_in_view(const View &view, const NodeBase *node) {
  return to_string((long long unsigned)node) + " : " +
         (view.viewData.count(const_cast<NodeBase *>(node)) ? "true" : "false");
}

string edge_in_view(const View &view, const EdgeBase *edge) {
  return node_in_view(view, edge->tail) + " --> " +
         node_in_view(view, edge->head);
}

vector<LineSegment> PhysicalNeighborhood(const View &view,
                                         const NodeBase &node) {
  vector<LineSegment> result;
  vector<EdgeBase *> neighborhood;
  copy_if(node.neighborhood.outgoing.begin(), node.neighborhood.outgoing.end(),
          back_inserter(neighborhood), [&view](const EdgeBase *edge) {
            return view.logicalSubView.count(edge->head);
          });
  copy_if(node.neighborhood.incoming.begin(), node.neighborhood.incoming.end(),
          back_inserter(neighborhood), [&view](const EdgeBase *edge) {
            return view.logicalSubView.count(edge->tail);
          });

  transform(
      neighborhood.begin(), neighborhood.end(), back_inserter(result),
      [&view](const EdgeBase *edge) { return PhysicalEdge(view, *edge); });
  return result;
}

void prune_isolated_nodes(View &view) {
  auto find_next_isolated = [](const ViewData::value_type &vt) {
    return vt.first->is_isolated();
  };
  for (auto kv = find_if(view.viewData.begin(), view.viewData.end(),
                         find_next_isolated);
       kv != view.viewData.end();
       kv = find_if(kv, view.viewData.end(), find_next_isolated)) {
    DIAGNOSTIC << "pruning: " << kv->first << endl;
    //view.viewData.erase(kv++);
    kv++;
  }
}

void set_logicalView(View &view, const std::vector<NodeBase *> &nodes) {
  view.logicalSubView.clear();
  copy(nodes.begin(), nodes.end(),
       inserter(view.logicalSubView, view.logicalSubView.begin()));
}

void set_physicalView(View &view, const Rectangle &view_box) {
  view.physicalSubView.nodes.clear();
  copy_if(
      view.logicalSubView.begin(), view.logicalSubView.end(),
      inserter(view.physicalSubView.nodes, view.physicalSubView.nodes.begin()),
      [&view, &view_box](const NodeBase *node) {
        auto neighborhood = PhysicalNeighborhood(view, *node);
        return any_of(neighborhood.begin(), neighborhood.end(),
                      [&view_box](const LineSegment &physical_edge) {
                        return view_box.Intersects(physical_edge);
                      }) ||
               view_box.Intersects(
                   *view.viewData.at(const_cast<NodeBase *>(node)).box);
      });
  view.physicalSubView.force_recalculate = false;
}

/*
:let my_matches = []:hi my_group ctermbg=blue
:let my_matches += ["=expand("<cword>")"]:match my_group /=join(my_matches,
"\\|")/ :let my_matches = my_matches[1:=len(my_matches)]:match my_group
/=join(my_matches, "\\|")/ :let @/ = "=join(my_matches, "\\\\|")":match
my_group /=join(my_matches, "\\|")/
*/
