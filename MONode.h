// FILENAME: MONode.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Header for MONode.C
// describes the structure of a monode (minimum overlap node) 
// of the hybrid tree.
// ------------------------------------------------------------


#ifndef MONODE_H
#define MONODE_H

#include "decl.h"
#include "Node.h"
#include "DataNode.h"
#include "OFNode.h"
#include "Point.h"
#include "Rect.h"
#include "Query.h"



struct OverlappingkdTreeInternalNode
{
        unsigned short splitDim;  /* dimension of split
                                     assumption is #dims < 256 */
        float splitPos1;           /* right boundary of the left region */
        float splitPos2;           /* left boundary of the right region */
        unsigned int left;        // index + MONODECARD if leaf, else index
        unsigned int right;       // in this way, we can directly say whether
                                  // the child is a leaf or not
        unsigned int parent;      // MONODECARD is stored in parent field for
                                  // root, all others have parent < MONODECARD
};

struct kdTreeLeafNode
{
  unsigned long count;
  // float radius;              /* center of live space BR used as centroid */
  unsigned int parent;
  Node *child;
};


#define OverlappingkdTreeInternalNodeSize sizeof(struct OverlappingkdTreeInternalNode)
#define kdTreeLeafNodeSize sizeof(struct kdTreeLeafNode)
#define MONODECARD (int)((PGSIZE-(4*sizeof(int)) + OverlappingkdTreeInternalNodeSize) /(OverlappingkdTreeInternalNodeSize + kdTreeLeafNodeSize))



class MONode : public Node {
 public:
  int numInternalNodes;   
  int numLeafNodes;   
  struct OverlappingkdTreeInternalNode internal[MONODECARD-1]; 
  struct kdTreeLeafNode leaf[MONODECARD]; 
 
  MONode();
  ~MONode();
  void PrintNode();
  void Init();
  void LeafInit(int i);
  void InternalInit(int i);
  void FillPickList(Point *r, int currRoot, struct kdTreeLeafInfo *picklist, int *pickcount, Rect *indexedRegion);
  int PickChild(Point *R, Rect *parentNodeRect, Rect *childNodeRect);
  int AddChild(Node *newChildNode, MONode **New_node, int kdTreeLeafIndex, struct split* childNodeSplit, struct split **kdTreeSplit, Rect *indexedRegion);

  // given the br of this node, get the BRs of all the children
  void FillChildList(int currRoot, Rect *childRectlist, Rect *indexedRegion); 
  // for bounding box search
  void FillVisitList(Rect *r, int currRoot, int *visitlist, int *visitcount);

    
  
  // splitting functions
  
  void GetkdtreeLeafBRs(int subtreeroot, Rect *indexedRegion, Rect *kdtreeLeafBRList);
  void GetNumLeavesInMOSubTree(int index, int *numLeaves, unsigned int *listOfLeaves);
  void GetMOSplitInfo(unsigned int subtreeroot, unsigned int stopnode, unsigned short splitdim, float splitpos1, float splitpos2, struct MOnode_split_info *newSplitInfo);
  struct MOnode_split_info GetMONodeSplitInfo(int index);
  int IsAncestor(unsigned int internalNodeIndex, unsigned int leafNodeIndex);
  struct numleaves GetNumLeavesInSubtree(struct MOnode_split_info *splitInfo, unsigned int subtreeroot, int option);
  int GetNextMOkdTreeNode(int option, int currnodeptr, struct MOnode_split_info *splitInfo);
  void ConstructMOSubtree(MONode *subtreeNode, struct MOnode_split_info *splitInfo, int option);
  void SplitMONodekdTree(MONode **nn, struct MOnode_split_info *splitInfo);
  void AssessSplits(struct MOnode_split_info *split_choice_array, int numPossibleSplitDims, Rect *kdtreeLeafBRList, Rect *indexedRegion);
  void GuaranteedAssessSplits(struct MOnode_split_info *split_choice_array, int numPossibleSplitDims, Rect *kdtreeLeafBRList, Rect *indexedRegion);
  void SplitNode(MONode **nn, struct split **newSplit, Rect *indexedRegion);
  
  /* function to create the first MONode, the root */
  
  void CreatekdTree(Node *leftChildNode, struct Node *rightChildNode, struct split *newSplit);
  int WhichChildOfParent(unsigned int index, int option);
  unsigned int GetSibling(unsigned int index, int option);
  int RemoveLeaf(int i);
  int Merge(MONode *sibling, struct OverlappingkdTreeInternalNode *newroot, int siblingWhichSide);
  void TransferFrom(int ptr, MONode *src);
  void CopyFrom(MONode *);

  // for multipoint queries
  // for NN search
  void FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, float (*User_Distance) (Point *, Point *));
  void FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, int dist_func);

  // for range search
  void FillVisitList(MultiPointQuery *MPQ, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, int dist_func);
  void FillVisitList(MultiPointQuery *MPQ, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, float (*User_Distance) (Point *, Point *));
  // end multipoint queries
};


struct MOnode_split_info
{
  // unsigned int splitNode;   // index of internal node used to split the kd-tree
  unsigned short splitdim;
  int numCascadingSplits;   // number of cascading splits
  float splitpos1;          // new split positions if this
  float splitpos2;          // split is performed
  float amountOfOverlap;    // amount of overlap
  unsigned int listOfSplittingChildren[MONODECARD]; // list of chilren who would split
  int numLeftChildren;      // left children (excluding replicated nodes)
  unsigned int listOfLeftChildren[MONODECARD];
  int numRightChildren;     // right children (excluding replicated nodes)
  unsigned int listOfRightChildren[MONODECARD];
};
#endif
