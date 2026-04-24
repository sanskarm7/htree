// FILENAME: HTree.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti 
// Header for HTree.C 
// describes the structure of the hybrid tree.
// ------------------------------------------------------------


#ifndef HTREE_H
#define HTREE_H

#include "decl.h"
#include "Node.h"
#include "DataNode.h"
#include "OFNode.h"
#include "MONode.h"
#include "Point.h"
#include "Rect.h"
#include "Query.h"


class HTree{
 public:

  Node *HTreeRoot;
  int height;
  Rect rootRect;
  FILE *fp;        // the HTree file, not the data file
  int firstitem; // is this the first data item being inserted
  int elsvalid;    
  int splitfreq[NUMDIMS]; // to verify of claim of implicit dim reduction
  int numemptynodes;     // to find out whether there are any empty nodes
  NODELIST pages_in_memory;  // list of pages in memory


  // creation, insertion, deletion, construct els etc. 
  // HTree.C contains these
  HTree();
  ~HTree();
  HTree(char *dumpfile); // loading from disk
  HTree(char *datafile, int max_inserts); // constructing from a data file
  int Load(Node *N, FILE *fp);
  void HTreeDump(char *dumpfile);
  int Dump(Node *N, FILE *fp);


   // insertion code

  int HTreeInsert2(Point *r, int tid, Node *n, Node **new_node, struct split **newsplit, int level, Rect *parentNodeRect, int cascading_splits_allowed, ReinsertItem *reinsertList, int *numItems);
  int HTreeInsert(Point *R, int Tid, Node **Root, int Level, int cascading_splits_allowed=0, ReinsertItem *reinsertList=NULL, int *numItems=NULL);
  int Insert(Point *R, int Tid);

  // deletion
  int Delete(Point *P, int Tid);
  int HTreeDeleteRect2(Point *P, int Tid, Node *N, int *isUnderFull, ReinsertList *l);
  // misc

  void ELS();
  Rect HTreeCreateELSs(Node *n, Rect *indexedRegion, unsigned long *objectcount);
  int IsPageFault(Node *n);
  void ClearBuffer();

  // end of functions in HTree.C

  // THE ONLY PUBLIC INTERFACE FOR QUERYING THE HYBRID TREE
  int Scan(Query *query);  // the one and only interface to search the hybrid tree

  // PRIVATE, NOT TO BE USED BY USER, RANGE QUERIES

  // RANGE QUERIES, contained in HTreeRangeSearch.C
  int Search(Rect *R, long *diskacc, Result *res, int max_returned);       // bounding box range query
  int Search(MultiPointQuery *mpq, float range, int dist_func, long *diskacc, Result *res, int max_returned); // distance based range query, using one of the in-built dist funcs
  int Search(MultiPointQuery *mpq, float range, float (*User_Distance) (Point *, Point *), long *diskacc, Result *res, int max_returned); // distance based range query, dist func specified by user

  // DISTANCE BASED SEARCH
  int HTreeSearch(Node *N, MultiPointQuery *mpq, float distance, int dist_func, long *diskacc, Rect *indexedRegion, Result *res, int max_returned);
  int HTreeSearch(Node *N, MultiPointQuery *mpq, float distance, float (*User_Distance) (Point *, Point *), long *diskacc, Rect *indexedRegion, Result *res, int max_returned);
  // BOUNDING BOX BASED SEARCH
  int HTreeSearch(Node *N, Rect *R, long *diskacc, Result *res, int max_returned);

  // end of functions related to RANGE QUERIES

  // NN QUERIES

  // NN query interface for multipoint queries
  int GetkNext(PQ *queue, MultiPointQuery *MPQ, int dist_func, long *diskacc, int k, Result *res); // k nearest neighbor 
  int GetkNext(PQ *queue, MultiPointQuery *MPQ, float (*User_Distance) (Point *, Point *), long *diskacc, int k, Result *res); // k nearest neighbor 
  // end of NN QUERIES, no feedback
};

#endif

