// graph_layout_algorithms.cc

#include "myassert.h"
#include "graph_layout_algorithms.h"

#include <algorithm>
#include <numeric>
#include <stack>
#include <unordered_map>
#include <vector>

#include <iostream>

using namespace std;

struct Grid {
  int row;
  int column;
  Grid(int row, int column) : row(row), column(column) {}
  Grid() : row(0), column(0) {}
};

using GridMap = unordered_map<NodeBase *, Grid>;

/*
 * Root is assumed to be "main()"
 *
 * (Deprecated, View now has explicit roots)
 */
void init_stack(stack<NodeBase *> &node_stack, const View &view,
                GridMap &gridMap) {
  auto kv =
      find_if(view.name_to_node->begin(), view.name_to_node->end(),
              [](const unordered_map<Fullname, NodeBase *>::value_type &kv) {
                return kv.first == "main()" || kv.first == "main(int, char **)";
              });
  if (kv == view.name_to_node->end()) {
    cout << "couldn't find node main()" << endl;
    return;
  }
  node_stack.push(kv->second);
  gridMap[kv->second] = Grid(0, 0);
}

/*
 * When the stack is empty, look for a new root
 */
bool refresh_stack(stack<NodeBase *> &node_stack, const View &view,
                   GridMap &gridMap, int &max_row) {
  for (auto &&node : view.roots) {
    // not visited
    if (!gridMap.count(node)) {
      node_stack.push(node);
      gridMap[node] = Grid(max_row++, 0);
      DIAGNOSTIC << "pushing root: " << view.viewData.at(node).text << endl;
      return true;
    }
  }
  return false;
}

/*
 * maximum row value and maximum column value are returned (it is useful for the
 * rest of the layout algorithm)
 */
pair<int, int> set_grid_dfs(const View &view, GridMap &gridMap) {
  // the gridMap keeps track of which nodes have been visited
  stack<NodeBase *> node_stack;
  int max_row = 0;
  int max_column = 0;

  // init_stack(node_stack, view, gridMap);

  while (!node_stack.empty() ||
         refresh_stack(node_stack, view, gridMap, max_row)) {
    const auto &outgoing = node_stack.top()->neighborhood.outgoing;
    int next_column = gridMap[node_stack.top()].column + 1;
    auto next = find_if(outgoing.begin(), outgoing.end(),
                        [&view, &gridMap](EdgeBase *edge) {
                          // if a node is not in the gridMap, it hasn't been
                          // visited
                          return view.logicalSubView.count(edge->head) &&
                                 !gridMap.count(edge->head);
                        });
    if (next == outgoing.end()) {
      node_stack.pop();
    } else {
      if (next != outgoing.begin()) {
        ++max_row;
      }
      node_stack.push((*next)->head);
      gridMap[(*next)->head] = Grid(max_row, next_column);
      max_column = max<int>(next_column, max_column);
    }
  }
  return {max_row, max_column};
}

/*
 * go through setting row/column width
 */
void get_row_column_dimensions(const View &view, const GridMap &gridMap,
                               vector<double> &row_height,
                               vector<double> &column_width) {
  for (auto &&node : view.logicalSubView) {
    // set max col/row...
    const double &node_height = view.viewData.at(node).box->extent.y;
    const double &node_width = view.viewData.at(node).box->extent.x;
    const int &row = gridMap.at(node).row;
    const int &column = gridMap.at(node).column;
    row_height[row] = max<double>(row_height[row], node_height);
    column_width[column] = max<double>(column_width[column], node_width);
  }
}

/*
 * calculate the positions based of results of rest of layout algorithm
 */
void set_initial_positions(View &view, const GridMap &gridMap,
                           const vector<double> &row_height,
                           const vector<double> &column_width) {
  for (auto &node : view.logicalSubView) {
    const int &row = gridMap.at(node).row;
    const int &column = gridMap.at(node).column;
    double row_pos = 0;
    double column_pos = 0;
    if (row != 0) {
      row_pos =
          accumulate(row_height.begin(), next(row_height.begin(), row), 0.0);
      row_pos += row * view.row_spacing;
    }
    if (column != 0) {
      column_pos = accumulate(column_width.begin(),
                              next(column_width.begin(), column), 0.0);
      column_pos += column * view.column_spacing;
      DIAGNOSTIC << view.viewData.at(node).text << ", row: " << row
                 << ", row_pos: " << row_pos << ", column: " << column
                 << ", column_pos: " << column_pos << endl;
    }
    /*
     * Here, notice that column_pos is the first coordinate and row_pos is the
     * second, this is because with position we go from the "row, column"
     * semantics of the grid to the "x,y" semantics of cartesian coordinates
     */
    view.viewData.at(node).box->position.x = column_pos;
    view.viewData.at(node).box->position.y = row_pos;
  }
}

/*
 * Create a view of the entire graph with nodes assigned grid locations DFS wise
 */
void get_initialViewFullGraph(View &view) {
  vector<double> row_height;
  vector<double> column_width;
  unordered_map<NodeBase *, Grid> gridMap;
  // calculate node grid positions, get the maximum row and column values
  pair<int, int> max_row_column = set_grid_dfs(view, gridMap);
  row_height.resize(max_row_column.first + 1);
  column_width.resize(max_row_column.second + 1);
  get_row_column_dimensions(view, gridMap, row_height, column_width);
  set_initial_positions(view, gridMap, row_height, column_width);
}

/*
 * Add node's children to graph and return the new view (new view created for
 * animation)
 */
View expand_node(View &view, NodeBase *node) {
  DIAGNOSTIC << "expanding node: " << &node << endl;
  auto box = *view.viewData.at(node).box;
  for (auto edge : node->neighborhood.outgoing) {
    view.logicalSubView.insert(edge->head);
    view.viewData.at(edge->head).box->position = box.position;
  }
  return view;
}
