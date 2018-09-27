// node.h
#pragma once

#include "node_base.h"

#include <iostream>

struct Edge;

struct SourceLocation {
  std::string filename;
  size_t line;
  size_t column;
  size_t offset;

  SourceLocation() : line(0), column(0), offset(0) {}
  SourceLocation(const std::string &filename, size_t line, size_t column,
                 size_t offset)
      : filename(filename), line(line), column(column), offset(offset) {}
};

struct SourceRange {
  SourceLocation begin;
  SourceLocation end;
};

class Node : public NodeBase {
public:
  Fullname fullname;
  SourceRange range;
  Node(Fullname fullname) : fullname(fullname) {}
  ~Node() {}
};

class Edge : public EdgeBase {
public:
  SourceRange range;
  Edge(Node *tail, Node *head) : EdgeBase(tail, head) {}
  ~Edge() {}
};

std::ostream &operator<<(std::ostream &, const SourceLocation &);
std::ostream &operator<<(std::ostream &, const SourceRange &);
std::ostream &operator<<(std::ostream &, const Node &);
std::ostream &operator<<(std::ostream &, const Edge &);
