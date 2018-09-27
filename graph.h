// graph.h
#pragma once

#include "node.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Graph {
  std::vector<Node *> nodes;
  std::vector<Edge *> edges;
  /*
   * The following maps map to the base class pointers so that other components
   * can use these tables without knowing about the nodes.
   */
  std::unordered_map<Fullname, NodeBase *> name_to_node;
  std::unordered_map<Fullname, EdgeBase *> name_to_edge;

  // void dump(std::ostream & = std::cout) const;
  std::pair<Node *, bool> try_createNode(const Fullname &);
  std::pair<Edge *, bool> try_createEdge(Node *tail, Node *head);

  //gets nodes with in_degree == 0
  std::vector<NodeBase *> get_roots() const; 

  ~Graph();
};

void dump_call_graph(const Graph &graph, std::ostream &o = std::cout);
Graph parseCallGraphFromFile(const std::string &filename);
