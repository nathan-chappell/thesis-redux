// function_node.cc

#include "node.h"

using namespace std;

ostream &operator<<(ostream &o, const SourceLocation &sourceLocation) {
  return o << sourceLocation.filename << " " << sourceLocation.line << " "
           << sourceLocation.column << " " << sourceLocation.offset;
}

ostream &operator<<(ostream &o, const SourceRange &sourceRange) {
  return o << sourceRange.begin << " " << sourceRange.end;
}

ostream &operator<<(ostream &o, const Node &node) {
  o << boolalpha;
  return o << node.fullname << ": " << &node << ": " << node.degree() << ", "
           << node.is_isolated() << node.range;
  // return o << node.fullname << ": " << node.range;
}

ostream &operator<<(ostream &o, const Edge &edge) {
  Node *tail = dynamic_cast<Node *>(edge.tail);
  Node *head = dynamic_cast<Node *>(edge.head);
  return o << tail->fullname << " -> " << head->fullname << " " << edge.range;
}
