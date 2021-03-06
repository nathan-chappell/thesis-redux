// node_base.h
#pragma once

// header only file!

/*
 * This file will hold some common defintions, forward definitions, and some
 * interfaces.  The original intent of this file is to give the layout
 * algorithms access to what they need to create views, and no more.
 */

#include <iostream>
#include <list>
#include <string>

using Fullname = std::string;

class NodeBase;
class EdgeBase;

/*
 * Tempted to make an iterator for this... (don't do it!)
 */
struct Neighborhood {
  std::list<EdgeBase *> outgoing;
  std::list<EdgeBase *> incoming;
};

class NodeBase {
public:
  Neighborhood neighborhood;
  size_t in_degree() const { return neighborhood.incoming.size(); }
  size_t out_degree() const { return neighborhood.outgoing.size(); }
  size_t degree() const { return in_degree() + out_degree(); }
  bool is_isolated() const { return !degree(); }
  virtual ~NodeBase() = 0;
};

class EdgeBase {
public:
  NodeBase *tail;
  NodeBase *head;
  EdgeBase(NodeBase *tail, NodeBase *head) : tail(tail), head(head) {}
  // virtual ~EdgeBase() = 0;
  virtual ~EdgeBase(); // taking pure virtualness off for error throwing!
};

inline std::ostream& operator<<(std::ostream &o, const EdgeBase &e) {
  return o << e.tail << " --> " << e.head;
}

/*
 * Just learned this trick.  Force the Bases to be abstract with purevirtual
 * destructor, but defined inline here so it will compile
 */
inline NodeBase::~NodeBase() {}
inline EdgeBase::~EdgeBase() {}
