// graph.cc

#include "graph.h"
#include "node.h"

#include <cassert>
#include <fstream>
#include <regex>

using namespace std;

void dump_call_graph(const Graph &graph, ostream &o) {
  for (auto &&node : graph.nodes) {
    o << *node << endl;
    for (auto &&edge : node->neighborhood.outgoing) {
      o << "  calls " << dynamic_cast<Node *>(edge->head)->fullname
        << dynamic_cast<const Edge*>(edge)->range << endl;
    }
    for (auto &&edge : node->neighborhood.incoming) {
      o << "  is called from " << dynamic_cast<Node *>(edge->tail)->fullname
        << dynamic_cast<const Edge*>(edge)->range << endl;
    }
  }
}

/*
 * If the name is already in the graph, return the pointer, otherwise add it
 */
pair<Node *, bool> Graph::try_createNode(const Fullname &fullname) {
  auto &&kv = name_to_node.find(fullname);
  if (kv != name_to_node.end()) {
    return {dynamic_cast<Node*>(kv->second), false};
  }
  Node *node = new Node{fullname};
  nodes.push_back(node);
  name_to_node[node->fullname] = node;
  return {node, true};
}

/*
 * If the edge is already in the graph, return the pointer, otherwise add it
 */
pair<Edge *, bool> Graph::try_createEdge(Node *tail, Node *head) {
  assert(tail && head && __func__);
  auto kv = name_to_edge.find(tail->fullname + "," + head->fullname);
  if (kv != name_to_edge.end()) {
    return {dynamic_cast<Edge*>(kv->second), false};
  }
  Edge *edge = new Edge(tail, head);
  edges.push_back(edge);
  tail->neighborhood.outgoing.push_back(edge);
  head->neighborhood.incoming.push_back(edge);
  return {edge, true};
}

SourceRange get_sourceRangeFromMatch(const smatch &m) {
  SourceLocation begin = {m[2], stoull(m[3]), stoull(m[4]), stoull(m[5])};
  SourceLocation end = {m[6], stoull(m[7]), stoull(m[8]), stoull(m[9])};
  return {begin, end};
}

// file_ex allows for escaped spaces
const string &file_ex = "\\s*(\\S*|\\\\\\s)*\\s*";
const string &line_ex = "(\\d*)\\s*";
const string &column_ex = "(\\d*)\\s*";
const string &offset_ex = "(\\d*)\\s*";
const string &location_ex = file_ex + line_ex + column_ex + offset_ex;
const string &calls_ex = "\\s*calls\\s*";

const string &fn_name_ex = "\\s*\\$(.*)\\$";

const regex caller_ex(fn_name_ex + location_ex + location_ex);
const regex callee_ex(calls_ex + fn_name_ex + location_ex + location_ex);

Graph parseCallGraphFromFile(const std::string &filename) {
  Graph graph;
  smatch m;
  string line;
  size_t line_no = 0;
  Fullname caller;
  Fullname callee;
  SourceRange range;
  ifstream file(filename);

  while (file.good()) {
    ++line_no;
    getline(file, line);
    if (line.empty()) {
      // skip empty line
      continue;
    }
    if (regex_match(line, m, callee_ex)) {
      // found edge
      callee = m[1];
      range = get_sourceRangeFromMatch(m);
      auto caller_pair = graph.try_createNode(caller);
      auto callee_pair = graph.try_createNode(callee);
      auto edge_pair =
          graph.try_createEdge(caller_pair.first, callee_pair.first);
      edge_pair.first->range = range;
    } else if (regex_match(line, m, caller_ex)) {
      // found node
      caller = m[1];
      range = get_sourceRangeFromMatch(m);
      auto node_p = graph.try_createNode(caller);
      node_p.first->range = range;
    } else {
      cout << "error parsing line[" << line_no << "]: " << line << endl;
      return graph;
    }
  }
  return graph;
}

Graph::~Graph() {
  for (auto &&node : nodes) {
    delete node;
  }
  for (auto &&edge : edges) {
    delete edge;
  }
}
