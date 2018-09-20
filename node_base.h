// node_base.h
#pragma once

/*
 * This file will hold some common defintions, forward definitions, and some
 * interfaces.  The original intent of this file is to give the layout
 * algorithms access to what they need to create views, and no more.
 */

#include <list>
#include <string>

using Fullname = std::string;

class NodeBase;
class EdgeBase;

struct Neighborhood {
  std::list<EdgeBase *> outgoing;
  std::list<EdgeBase *> incoming;
};

class NodeBase {
public:
  Neighborhood neighborhood;
  size_t degree() const {
    return neighborhood.outgoing.size() + neighborhood.incoming.size();
  }
  bool isolated() const { return !degree(); }
  virtual ~NodeBase() = 0;
};

class EdgeBase {
public:
  NodeBase *tail;
  NodeBase *head;
  EdgeBase(NodeBase *tail, NodeBase *head) : tail(tail), head(head) {}
  virtual ~EdgeBase() = 0;
};

/*
 * Just learned this trick.  Force the Bases to be abstract with purevirtual
 * destructor, but defined inline here so it will compile
 */
inline NodeBase::~NodeBase() {}
inline EdgeBase::~EdgeBase() {}
