// FILENAME: DataNode.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti 
// Header for DataNode.C 
// describes the structure of a datanode of the hybrid tree. 
// ------------------------------------------------------------


#ifndef DATANODE_H
#define DATANODE_H

#include "decl.h"
#include "Node.h"
#include "Point.h"
#include "Rect.h"
#include "Query.h"

class Rect;
class Point;


struct dimvar
{
  unsigned short dimension;
  float variance;
  float mean;
};

struct dimspread
{
  unsigned short dimension;
  float spread;
};


struct ReinsertItem
{
  Point point;
  long tid;
};

struct Branch
{
  Point point;
  Node *child;    /* can either be pointer to node or leafnode */
};

#define NODECARD (int)((PGSIZE-(3*sizeof(int))) / sizeof(struct Branch))


class DataNode : public Node {
 public:
  int count;
  struct Branch branch[NODECARD];
  
  DataNode();
  ~DataNode();
  void Init();
  void BranchInit(int);
  int Count();
  void Count(int);
  void IncrCount();
  struct Branch Branch(int);
  void Branch(int, struct Branch);
  void DisconnectBranch(int i);
  void Print();
  Rect Cover(); 
  int AddChild(struct Branch *, DataNode**, struct split**);
  struct split MaxExtentBisectionSplit();
  void TransferFrom(DataNode *src);
  void FillReinsertList(ReinsertList *l);
  void SplitNode(DataNode**, struct split**);
  int ComputeNumReinsertions(unsigned int splitdimension, float splitposition, int *num_reinseriions);
  int ComputeReinsertions(unsigned int splitdimension, float splitposition, ReinsertItem *reinsertion_list, int *num_points_in_list);
  void DistributeDataItems(Node *, unsigned int , float );
  int IsEmptyPartition(unsigned int , float , int );
  void CopyFrom(DataNode *n);
  Rect CreateELS(unsigned int *ELSarray, Rect *totalspace);
  float Radius(Point *centroid);
};

#endif
