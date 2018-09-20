//function_node.cc

#include "node.h"

using namespace std;

ostream &operator<<(ostream &o, const SourceLocation &sourceLocation) {
  o << sourceLocation.filename << " " << sourceLocation.line << " "
    << sourceLocation.column << " " << sourceLocation.offset;
  return o;
}

ostream &operator<<(ostream &o, const SourceRange &sourceRange) {
  o << sourceRange.begin << " " << sourceRange.end;
  return o;
}

ostream &operator<<(ostream &o, const NodeBase &nodeBase) {
  const Node& node = dynamic_cast<const Node&>(nodeBase);
  o << node.fullname << " " << node.range;
  return o;
}

ostream &operator<<(ostream &o, const EdgeBase &edgeBase) {
  const Edge &edge = dynamic_cast<const Edge&>(edgeBase);
  Node *tail = dynamic_cast<Node*>(edge.tail);
  Node *head = dynamic_cast<Node*>(edge.head);
  o << tail->fullname << " -> " << head->fullname << " " << edge.range;
  return o;
}

