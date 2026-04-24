// FILENAME: Node.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// defines the node of the hybrid tree
// a node can be of any of the 3 types:DataNode, OFNode and MONode
// which are subclasses of this class
// type and level are member elements common to all the 3 subclasses
// ------------------------------------------------------------


#ifndef NODE_H
#define NODE_H


class Node{
 public:
  enum {DATANODE, OFNODE, MONODE};
  int type;                /* 1 if ordinary node, 2 if OFnode, 3 is MOnode */
  int level;               /* 0 is leaf, others positive */
  
  Node(int); 
  ~Node();
};


#endif
