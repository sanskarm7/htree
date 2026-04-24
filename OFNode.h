// FILENAME: OFNode.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Header for OFNode.C
// describes the structure of an ofnode (overlap free node i.e.
// the nodes of the level just above the leaf level) of the hybrid tree.
// nodes of this level are overlap-free because when they split,
// they cause downward cascading splits to the next lower level 
// (the data node level). this may lead to violation of storage
// utilization but overall, in average, it gives good performance
// ------------------------------------------------------------

#ifndef OFNODE_H
#define OFNODE_H

#include "decl.h"
#include "Node.h"
#include "DataNode.h"
#include "Point.h"
#include "Rect.h"
#include "Query.h"
#include "MultiPointQuery.h"

struct kdTreeLeafNodewithELS
{
  Node *child;
  unsigned int parent;
  unsigned int count;
  float radius;              /* center of live space BR used as centroid */
  unsigned short elsvalid;
  unsigned int elsarray[ELSARRAYLEN];
};

struct kdTreeInternalNode
{
  unsigned short splitDim;  /* dimension of split
                               assumption is #dims < 256 */
  float splitPos1;           /* position of split */
  float splitPos2;
  unsigned int left;        /* left child */
  unsigned int right;       /* right child */
                            /* Convention: array_index + OFNODECARD if
                               child is leaf, else just array_index, in
                               this way, we can directly say whether
                               the child is a leaf or not  */
  unsigned int parent;      /* OFNODECARD is stored in parent field for
                               root, all others have parent < OFNODECARD */
};

#define kdTreeInternalNodeSize sizeof(struct kdTreeInternalNode)
#define kdTreeLeafNodewithELSSize sizeof(struct kdTreeLeafNodewithELS)
#define OFNODECARD (int)((PGSIZE-(4*sizeof(int)) + kdTreeInternalNodeSize)/(kdTreeInternalNodeSize + kdTreeLeafNodewithELSSize))
#define MAXCASCADINGSPLITS (OFNODECARD/4)


class OFNode : public Node {
 public:
  int numInternalNodes;   
  int numLeafNodes;   
  struct kdTreeInternalNode internal[OFNODECARD-1]; 
  struct kdTreeLeafNodewithELS leaf[OFNODECARD]; 
  
  OFNode();
  ~OFNode();
  void InternalInit(int i);
  void LeafInit(int i);
  int NumInternalNodes();
  void NumInternalNodes(int newnum);
  void IncNumInternalNodes();
  int NumLeafNodes();
  void NumLeafNodes(int newnum);
  void IncNumLeafNodes();
  struct kdTreeInternalNode Internal(int i);
  void Internal(int i, struct kdTreeInternalNode I);
  void InternalLeft(int i, int leftchildindex);
  void InternalRight(int i, int rightchildindex);
  void InternalParent(int i, int parentindex);
  struct kdTreeLeafNodewithELS Leaf(int i);
  void Leaf(int i, struct kdTreeLeafNodewithELS L);
  void LeafParent(int i, int parentindex);
  
  void Print();
  Rect Cover(Rect*, struct split*, int); 
  int AddChild(Node *newDataNode, OFNode **New_node, int kdTreeLeafIndex, struct split* dataNodeSplit, struct split** kdTreeSplit, Rect* indexedRegion, int cascading_splits_allowed, ReinsertItem *reinsertList, int *numItems); 
  /* add the DataNode, produced due to splitting of the leaf (3rd argument) */
  void FillPickList(Point *r, int currRoot, struct kdTreeLeafInfo *picklist, int *pickcount, Rect *indexedRegion);
  int PickChild(Point *R, Rect *parentNodeRect, Rect *childNodeRect); /* return the index the child the point should be inserted into */
 
  // given the br of this node, get the BRs of all the children
  void FillChildList(int currRoot, Rect *childRectlist, Rect *indexedRegion); 
  /* for bounding box queries, r is the bounding box */
  void FillVisitList(Rect *r, int currRoot, int *visitlist, int *visitcount);

  /* functions to split an OFNode */
  int AnalyzeSplit(int splitnode);
  void GetkdtreeLeafBRs(int subtreeroot, Rect *indexedRegion, Rect *kdtreeLeafBRList);
  void GetNumLeavesInOFSubTree(int index, int *numLeaves, unsigned int *listOfLeaves);
  void GetOFSplitInfo(unsigned int subtreeroot, unsigned int stopnode, unsigned short splitdim, float splitpos1, float splitpos2, struct OFnode_split_info *newSplitInfo);
  struct OFnode_split_info GetOFNodeSplitInfo(int index);
  int IsAncestor(unsigned int internalNodeIndex, unsigned int leafNodeIndex);
  struct numleaves GetNumLeavesInSubtree(struct OFnode_split_info *splitInfo, unsigned int subtreeroot, int option);
  int GetNextOFkdTreeNode(int option, int currnodeptr, struct OFnode_split_info *splitInfo); 
  void ConstructOFSubtree(OFNode *subtreeNode, struct OFnode_split_info *splitInfo, int option);
  void SplitOFNodekdTree(OFNode **nn, struct OFnode_split_info *splitInfo);
  void GuaranteedAssessSplits(struct OFnode_split_info *split_choice_array, int numPossibleSplitDims, Rect *kdtreeLeafBRList, Rect *indexedRegion);

  int GetNumLeavesInSubTree(int index);
  void GetSplitInfo(unsigned int subtreeroot, unsigned int stopnode, unsigned short splitdim, float splitpos, struct split_info *newSplitInfo);
  void GetListOfSplittingDataNodes(unsigned int subtreeroot, unsigned int stopnode, unsigned short splitdim, float splitpos, unsigned int *list, int *listcount);
  struct split_info GetkdTreeSplitInfo(int index);
  int GetNextNode(int splitnode, int option, int currnodeptr);
  void ConstructSubtree(OFNode *subtreeNode, int splitnode, int option, DataNode **deleted_child_nodes=NULL, int *num_deleted_child_nodes=NULL, ReinsertItem *reinsertion_list=NULL, int *num_points_in_list=NULL);
  unsigned int GetSibling(unsigned int index, int option);
  int WhichChildOfParent(unsigned int index, int option);
  int RemoveLeaf(int i);
  int EliminateEmptyNodes(DataNode **deleted_child_nodes=NULL, int num_deleted_child_nodes=0);
  int Merge(OFNode *sibling, struct kdTreeInternalNode *newroot, int siblingWhichSide);
  void TransferFrom(int ptr, OFNode *src);
  void SplitkdTree( OFNode **nn, int splitnode, int num_cascading_splits, ReinsertItem *reinsertList, int *numItems);
  void SplitNode(OFNode **nn, struct split **newSplit, Rect *indexedRegion, int cascading_splits_allowed, ReinsertItem *reinsertList, int *numItems);

  /* function to create the first OFNode, the root */

  void CreatekdTree(Node*, Node*, struct split*);
  void CopyFrom(OFNode *);
  // multipoint queries
  // for NN search
  void FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, int dist_func);
  void FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, float (*User_Distance) (Point *, Point *));

  // for range search
  void FillVisitList(MultiPointQuery *MPQ, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, int dist_func);
  void FillVisitList(MultiPointQuery *MPQ, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, float (*User_Distance) (Point *, Point *));
  // end multipoint queries
};

struct OFnode_split_info
{
  // unsigned int splitNode;   // index of internal node used to split the kd-tree
  unsigned short splitdim;
  int numCascadingSplits;   // number of cascading splits
  float splitpos1;          // new split positions if this
  float splitpos2;          // split is performed
  float amountOfOverlap;    // amount of overlap
  unsigned int listOfSplittingChildren[OFNODECARD]; // list of chilren who would split
  int numLeftChildren;      // left children (excluding replicated nodes)
  unsigned int listOfLeftChildren[OFNODECARD];
  int numRightChildren;     // right children (excluding replicated nodes)
  unsigned int listOfRightChildren[OFNODECARD];
};


struct split_info
{
  unsigned int splitNode;   // index of internal node used to split the kd-tree
  int numCascadingSplits;   // number of cascading splits
  int numLeftChildren;      // left children (excluding replicated nodes)
  int numRightChildren;     // right children (excluding replicated nodes)
};

#endif
  
