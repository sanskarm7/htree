// FILENAME: OFNode.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the OFNode of the hybrid tree.
// implements functions to add a child to the ofnode,
// split the ofnode in case of an overflow etc.
// ------------------------------------------------------------

#include "OFNode.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// some misc functions to check presence of an element in a set,
// comparison functions for structs (so can we can pass it to qsort etc.

int IsASplittingLeaf(unsigned int aLeaf, unsigned int *splittingLeaves,
                     int splittingLeafCount) {
  int i;

  for (i = 0; i < splittingLeafCount; i++)
    if (splittingLeaves[i] == aLeaf)
      return 1;

  return 0;
}

int splitcmp(struct split_info *a, struct split_info *b) {
  if (a->numCascadingSplits < b->numCascadingSplits)
    return -1;
  else if (a->numCascadingSplits == b->numCascadingSplits)
    return 0;
  else
    return 1;
}

int leftboundcmp2(struct lineseg *a, struct lineseg *b) {
  if (a->leftbound < b->leftbound)
    return -1;
  else if (a->leftbound == b->leftbound)
    return 0;
  else
    return 1;
}

int rightboundcmp2(struct lineseg *a, struct lineseg *b) {
  if (a->rightbound > b->rightbound)
    return -1;
  else if (a->rightbound == b->rightbound)
    return 0;
  else
    return 1;
}

int overlapcmp2(struct OFnode_split_info *a, struct OFnode_split_info *b) {
  if (a->amountOfOverlap < b->amountOfOverlap)
    return -1;
  else if (a->amountOfOverlap == b->amountOfOverlap)
    return 0;
  else
    return 1;
}

int IsPresent(DataNode *aChild, DataNode **childList, int childCount) {
  int i;
  for (i = 0; i < childCount; i++)
    if (childList[i] == aChild)
      return 1;
  return 0;
}

int IsPresent2(unsigned int aLeaf, unsigned int *leafList, int leafCount) {
  int i;

  for (i = 0; i < leafCount; i++)
    if (leafList[i] == aLeaf)
      return 1;
  return 0;
}

// end of misc functions

// constructor

OFNode::OFNode() : Node(Node::OFNODE) {
  int i;
  numInternalNodes = 0;
  numLeafNodes = 0;
  for (i = 0; i < OFNODECARD - 1; i++)
    InternalInit(i);
  for (i = 0; i < OFNODECARD; i++)
    LeafInit(i);
}

// destructor

OFNode::~OFNode() {}

// initialize functions
void OFNode::LeafInit(int i) {
  leaf[i].parent = 0;
  leaf[i].elsvalid = 0;
  leaf[i].radius = 0.0;
  leaf[i].child = NULL;
}

void OFNode::InternalInit(int i) {
  internal[i].splitDim = 0;
  internal[i].splitPos1 = 0.0;
  internal[i].splitPos2 = 0.0;
  internal[i].left = 0;
  internal[i].right = 0;
}

// accessor functions

int OFNode::NumInternalNodes() { return numInternalNodes; }

void OFNode::NumInternalNodes(int newnum) { numInternalNodes = newnum; }

void OFNode::IncNumInternalNodes() { numInternalNodes++; }

int OFNode::NumLeafNodes() { return numLeafNodes; }

void OFNode::NumLeafNodes(int newnum) { numLeafNodes = newnum; }

void OFNode::IncNumLeafNodes() { numLeafNodes++; }

struct kdTreeInternalNode OFNode::Internal(int i) {
  assert(i < numInternalNodes);
  return internal[i];
}

void OFNode::Internal(int i, struct kdTreeInternalNode I) { internal[i] = I; }

void OFNode::InternalLeft(int i, int leftchildindex) {
  internal[i].left = leftchildindex;
}

void OFNode::InternalRight(int i, int rightchildindex) {
  internal[i].right = rightchildindex;
}

void OFNode::InternalParent(int i, int parentindex) {
  internal[i].parent = parentindex;
}

struct kdTreeLeafNodewithELS OFNode::Leaf(int i) {
  assert(i < numLeafNodes);
  return leaf[i];
}

void OFNode::Leaf(int i, struct kdTreeLeafNodewithELS L) { leaf[i] = L; }

void OFNode::LeafParent(int i, int parentindex) {
  leaf[i].parent = parentindex;
}

void OFNode::Print() {
  // fill in printing stuff with proper indentation
  printf("in print");
}

// given a rectangle and a split, partitions the rectange into 2 parts,
// returns the partition desired, 0 for the lower side partition
// 1 for the higher side partition
Rect OFNode::Cover(Rect *R, struct split *newsplit, int whichSide) {
  Rect *r = R;
  Rect newRect;
  int i;

  newRect = *r;

  if (whichSide == 0) // lower side
    newRect.High(newsplit->splitDim, newsplit->splitPos);
  else                                                    // higher side
    newRect.Low(newsplit->splitDim, newsplit->splitPos2); // changed
  return newRect;
}

// add a child -- splits if full

int OFNode::AddChild(Node *newDataNode, OFNode **New_node, int kdTreeLeafIndex,
                     struct split *dataNodeSplit, struct split **kdTreeSplit,
                     Rect *indexedRegion, int cascading_splits_allowed,
                     ReinsertItem *reinsertList, int *numItems) {
  OFNode **new_node = New_node;
  int i, parent;

  parent = leaf[kdTreeLeafIndex].parent;
  if (internal[parent].left == (kdTreeLeafIndex + OFNODECARD))
    internal[parent].left = numInternalNodes;
  else if (internal[parent].right == (kdTreeLeafIndex + OFNODECARD))
    internal[parent].right = numInternalNodes;
  else
    assert(FALSE); // shouldnt happen

  leaf[kdTreeLeafIndex].parent = numInternalNodes;

  internal[numInternalNodes].splitDim = dataNodeSplit->splitDim;
  internal[numInternalNodes].splitPos1 = dataNodeSplit->splitPos;
  internal[numInternalNodes].splitPos2 = dataNodeSplit->splitPos2;
  // this assertion is true for point data
  assert(internal[numInternalNodes].splitPos1 ==
         internal[numInternalNodes].splitPos2);
  internal[numInternalNodes].left = OFNODECARD + kdTreeLeafIndex;
  internal[numInternalNodes].right = OFNODECARD + numLeafNodes;
  internal[numInternalNodes].parent = parent;
  leaf[numLeafNodes].child = newDataNode;
  leaf[numLeafNodes].parent = numInternalNodes;
  // invalidate the els
  leaf[numLeafNodes].elsvalid = 0;

  numInternalNodes++;
  numLeafNodes++;

  if (numLeafNodes >= OFNODECARD) // the OFnode is full, should be split
  {
    SplitNode(new_node, kdTreeSplit, indexedRegion, cascading_splits_allowed,
              reinsertList, numItems);
    return 1;
  }

  else
    return 0;
}

// changed pickchild
void OFNode::FillPickList(Point *r, int currRoot,
                          struct kdTreeLeafInfo *picklist, int *pickcount,
                          Rect *indexedRegion) {
  int i;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  intNode = &(internal[currRoot]);
  if (r->position[intNode->splitDim] <=
      intNode->splitPos1) // point to be inserted
                          // overlaps with left partition
  {
    leftIndexedRegion = *indexedRegion;
    leftIndexedRegion.boundary[NUMDIMS + (intNode->splitDim)] =
        intNode->splitPos1;
    if (intNode->left >= OFNODECARD) {
      picklist[*pickcount].leafNodeIndex = (intNode->left) - OFNODECARD;
      picklist[*pickcount].leafRect = leftIndexedRegion;
      (*pickcount)++;
    } else {
      FillPickList(r, intNode->left, picklist, pickcount, &leftIndexedRegion);
    }
  }

  if (r->position[intNode->splitDim] >
      intNode->splitPos2) // point to be inserted
                          // overlaps with right partition
  {
    rightIndexedRegion = *indexedRegion;
    rightIndexedRegion.boundary[intNode->splitDim] = intNode->splitPos2;
    if (intNode->right >= OFNODECARD) {
      picklist[*pickcount].leafNodeIndex = (intNode->right) - OFNODECARD;
      picklist[*pickcount].leafRect = rightIndexedRegion;
      (*pickcount)++;
    } else {
      FillPickList(r, intNode->right, picklist, pickcount, &rightIndexedRegion);
    }
  }
}

// Pick the branch in which the point should be inserted.
// Pick the one which needs minimum enlargement.
// Returns the index of the leafnode of the kdtree into
// which the point should be inserted
//
int OFNode::PickChild(Point *R, Rect *parentNodeRect, Rect *childNodeRect) {
  Point *r = R;
  struct kdTreeLeafInfo picklist[OFNODECARD];
  int pickcount = 0;
  float minvol = 999.0;
  float volume;
  int i;
  int bestPick = -1; // kdtreeLeafNode index for the best pick
  FillPickList(r, 0, picklist, &pickcount, parentNodeRect);

  assert(pickcount >
         0); // at least there is one leaf overlapping with the point

  if (pickcount == 1) // point overlaps with only one node, this is the leaf
  {
    (*childNodeRect) = picklist[0].leafRect;
    assert(picklist[0].leafNodeIndex < numLeafNodes);
    return (picklist[0].leafNodeIndex);
  }

  else
    // pickcount > 1, point overlaps with many nodes, all need zero
    // enlargement,pick one of them arbitrarily
    bestPick = (int)(drand48() * pickcount);

  assert(bestPick >= 0);
  (*childNodeRect) = picklist[bestPick].leafRect;
  assert(picklist[bestPick].leafNodeIndex < numLeafNodes);
  return (picklist[bestPick].leafNodeIndex);
}

// get rect's of all children
void OFNode::FillChildList(int currRoot, Rect *childRectlist,
                           Rect *indexedRegion) {
  int i;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  if (numLeafNodes <= 0)
    return;
  intNode = &(internal[currRoot]);

  leftIndexedRegion = *indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS + (intNode->splitDim)] =
      intNode->splitPos1;

  if (intNode->left >= OFNODECARD) {
    childRectlist[(intNode->left) - OFNODECARD] = leftIndexedRegion;
  } else {
    FillChildList(intNode->left, childRectlist, &leftIndexedRegion);
  }

  rightIndexedRegion = *indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim] = intNode->splitPos2;

  if (intNode->right >= OFNODECARD) {
    childRectlist[(intNode->right) - OFNODECARD] = rightIndexedRegion;
  } else {
    FillChildList(intNode->right, childRectlist, &rightIndexedRegion);
  }
}

// for bounding box queries where r is the bounding box
void OFNode::FillVisitList(Rect *r, int currRoot, int *visitlist,
                           int *visitcount) {
  struct kdTreeInternalNode *intNode;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node
  intNode = &(internal[currRoot]);

  if (numLeafNodes <= 0) {
    *visitcount = 0;
    return;
  }
  // query rect. overlaps  with left partition
  if (r->Low(intNode->splitDim) < intNode->splitPos1) {
    if (intNode->left >= OFNODECARD) {
      visitlist[*visitcount] = (intNode->left) - OFNODECARD;
      (*visitcount)++;
    } else {
      FillVisitList(r, intNode->left, visitlist, visitcount);
    }
  }

  // query rect overlaps with right partition
  if (r->High(intNode->splitDim) > intNode->splitPos2) {
    if (intNode->right >= OFNODECARD) {
      visitlist[*visitcount] = (intNode->right) - OFNODECARD;
      (*visitcount)++;
    } else {
      FillVisitList(r, intNode->right, visitlist, visitcount);
    }
  }
}

// splitting functions for OFNode

// the following set of functions are to split a OFnode
// the MO node (the lowest level MO node is actually OF because the
// level below it is OF and a OF node is split in an OF manner)
// is split in a MO manner, there is no question of
// cascading splits. cascading splits is avoided because the cascade
// is no longer retricted to one level since there many levels below the
// level of MO node. we choose the split with minimum amount
// of overlap which satisfies the min. utilization

void OFNode::GetkdtreeLeafBRs(int subtreeroot, Rect *indexedRegion,
                              Rect *kdtreeLeafBRList) {
  Rect leftRegion, rightRegion;
  unsigned short thissplitDim;
  float leftsplitPos, rightsplitPos;
  if (subtreeroot >= OFNODECARD) {
    kdtreeLeafBRList[subtreeroot - OFNODECARD] = (*indexedRegion);
  } else /* internal node */
  {
    thissplitDim = internal[subtreeroot].splitDim;
    leftsplitPos = internal[subtreeroot].splitPos1;
    rightsplitPos = internal[subtreeroot].splitPos2;

    leftRegion = *indexedRegion;
    leftRegion.boundary[NUMDIMS + thissplitDim] = leftsplitPos;
    GetkdtreeLeafBRs(internal[subtreeroot].left, &leftRegion, kdtreeLeafBRList);

    rightRegion = *indexedRegion;
    rightRegion.boundary[thissplitDim] = rightsplitPos;
    GetkdtreeLeafBRs(internal[subtreeroot].right, &rightRegion,
                     kdtreeLeafBRList);
  }
}

void OFNode::GetNumLeavesInOFSubTree(int index, int *numLeaves,
                                     unsigned int *listOfLeaves) {
  if (index < OFNODECARD) /* internal node */
  {
    GetNumLeavesInOFSubTree(internal[index].left, numLeaves, listOfLeaves);
    GetNumLeavesInOFSubTree(internal[index].right, numLeaves, listOfLeaves);
  } else /* leaf node */
  {
    listOfLeaves[(*numLeaves)] = (index - OFNODECARD);
    (*numLeaves)++;
  }
}

void OFNode::GetOFSplitInfo(unsigned int subtreeroot, unsigned int stopnode,
                            unsigned short splitdim, float splitpos1,
                            float splitpos2,
                            struct OFnode_split_info *newSplitInfo) {
  if (subtreeroot >= OFNODECARD) {
    newSplitInfo->listOfSplittingChildren[newSplitInfo->numCascadingSplits] =
        subtreeroot - OFNODECARD;
    newSplitInfo->numCascadingSplits++;
  } else /* internal node */
  {
    if (internal[subtreeroot].splitDim != splitdim) {
      GetOFSplitInfo(internal[subtreeroot].left, stopnode, splitdim, splitpos1,
                     splitpos2, newSplitInfo);

      GetOFSplitInfo(internal[subtreeroot].right, stopnode, splitdim, splitpos1,
                     splitpos2, newSplitInfo);
    } else /* n->rest.monode.internal[subtreeroot].splitDim == splitdim */
    {
      if (internal[subtreeroot].splitPos1 <= splitpos2)
        GetNumLeavesInOFSubTree(internal[subtreeroot].left,
                                &(newSplitInfo->numLeftChildren),
                                newSplitInfo->listOfLeftChildren);
      else
        GetOFSplitInfo(internal[subtreeroot].left, stopnode, splitdim,
                       splitpos1, splitpos2, newSplitInfo);

      if (internal[subtreeroot].splitPos2 >= splitpos1)
        GetNumLeavesInOFSubTree(internal[subtreeroot].right,
                                &(newSplitInfo->numRightChildren),
                                newSplitInfo->listOfRightChildren);
      else
        GetOFSplitInfo(internal[subtreeroot].right, stopnode, splitdim,
                       splitpos1, splitpos2, newSplitInfo);
    }
  }
}

/* return split information i.e. number of cascading splits,
   utilization etc. if internal[index] is used as the split
   hyperplane
*/

struct OFnode_split_info OFNode::GetOFNodeSplitInfo(int index) {
  struct OFnode_split_info newSplitInfo;
  unsigned short splitdim = internal[index].splitDim;
  float splitpos1 = internal[index].splitPos1;
  float splitpos2 = internal[index].splitPos2;
  /* initialize */
  // newSplitInfo.splitNode=index;
  newSplitInfo.splitdim = splitdim;
  newSplitInfo.numCascadingSplits = 0;
  newSplitInfo.numLeftChildren = 0;
  newSplitInfo.numRightChildren = 0;

  if (index == 0) // root of the kd-tree
  {
    GetNumLeavesInOFSubTree(internal[index].left,
                            &(newSplitInfo.numLeftChildren),
                            newSplitInfo.listOfLeftChildren);
    GetNumLeavesInOFSubTree(internal[index].right,
                            &(newSplitInfo.numRightChildren),
                            newSplitInfo.listOfRightChildren);
  } else {
    GetOFSplitInfo(0, index, splitdim, splitpos1, splitpos2, &newSplitInfo);
  }
  assert((newSplitInfo.numRightChildren + newSplitInfo.numLeftChildren +
          newSplitInfo.numCascadingSplits) == OFNODECARD);
  return newSplitInfo;
}

int OFNode::IsAncestor(unsigned int internalNodeIndex,
                       unsigned int leafNodeIndex) {
  unsigned int ancestor;
  ancestor = leaf[leafNodeIndex].parent;

  for (;;) {
    if (ancestor >= OFNODECARD)
      break; // reached the root of the kd-tree
    if (ancestor == internalNodeIndex)
      return 1; // the internal node is an ancestor of the leaf
    ancestor = internal[ancestor].parent;
  }
  return 0;
}

struct numleaves
OFNode::GetNumLeavesInSubtree(struct OFnode_split_info *splitInfo,
                              unsigned int subtreeroot, int option) {
  struct numleaves leafInfo;
  int leaf, i;
  int newsubtreeroot;

  leafInfo.numLeavesInSubtree = 0;
  leafInfo.numLeavesUnderLeftChild = 0;

  if (option == LEFT) {
    for (i = 0; i < splitInfo->numLeftChildren; i++) {
      leaf = splitInfo->listOfLeftChildren[i];
      if (IsAncestor(subtreeroot, leaf)) {
        (leafInfo.numLeavesInSubtree)++;
        leafInfo.leafNode = leaf;
      }
    }

    if (leafInfo.numLeavesInSubtree > 1) {
      newsubtreeroot = internal[subtreeroot].left;
      if (newsubtreeroot >= OFNODECARD) {
        for (i = 0; i < splitInfo->numLeftChildren; i++) {
          if (splitInfo->listOfLeftChildren[i] ==
              (newsubtreeroot - OFNODECARD)) {
            leafInfo.numLeavesUnderLeftChild = 1;
            break;
          }
        }
      } else {
        for (i = 0; i < splitInfo->numLeftChildren; i++) {
          leaf = splitInfo->listOfLeftChildren[i];
          if (IsAncestor(newsubtreeroot, leaf))
            (leafInfo.numLeavesUnderLeftChild)++;
        }
      }
    }
  }

  if (option == RIGHT) {
    for (i = 0; i < splitInfo->numRightChildren; i++) {
      leaf = splitInfo->listOfRightChildren[i];
      if (IsAncestor(subtreeroot, leaf)) {
        (leafInfo.numLeavesInSubtree)++;
        leafInfo.leafNode = leaf;
      }
    }

    if (leafInfo.numLeavesInSubtree > 1) {
      newsubtreeroot = internal[subtreeroot].left;
      if (newsubtreeroot >= OFNODECARD) {
        for (i = 0; i < splitInfo->numRightChildren; i++) {
          if (splitInfo->listOfRightChildren[i] ==
              (newsubtreeroot - OFNODECARD)) {
            leafInfo.numLeavesUnderLeftChild = 1;
            break;
          }
        }
      } else {
        for (i = 0; i < splitInfo->numRightChildren; i++) {
          leaf = splitInfo->listOfRightChildren[i];
          if (IsAncestor(newsubtreeroot, leaf))
            (leafInfo.numLeavesUnderLeftChild)++;
        }
      }
    }
  }
  return leafInfo;
}

int OFNode::GetNextOFkdTreeNode(int option, int currnodeptr,
                                struct OFnode_split_info *splitInfo) {
  int ptr = currnodeptr;
  struct numleaves leafInfo;

  assert(ptr < (OFNODECARD - 1)); /* pointing to an internal node */

  for (;;) {
    if (ptr >= OFNODECARD)
      return ptr; /* pointing to a leaf node */

    if (option == LEFT) {
      if (internal[ptr].splitDim == splitInfo->splitdim) {
        if (internal[ptr].splitPos2 >= splitInfo->splitpos1) {
          ptr = internal[ptr].left;
          continue;
        }
        if (internal[ptr].splitPos2 < splitInfo->splitpos2)
          return ptr;
      }

      leafInfo = GetNumLeavesInSubtree(splitInfo, ptr, LEFT);
      if (leafInfo.numLeavesInSubtree == 1) {
        ptr = leafInfo.leafNode + OFNODECARD;
        return ptr;
      } else {
        if (leafInfo.numLeavesUnderLeftChild == 0)
          ptr = internal[ptr].right;

        else if (leafInfo.numLeavesUnderLeftChild ==
                 leafInfo.numLeavesInSubtree)
          ptr = internal[ptr].left;

        else
          return ptr;
      }
    }

    if (option == RIGHT) {

      if (internal[ptr].splitDim == splitInfo->splitdim) {
        if (internal[ptr].splitPos1 <= splitInfo->splitpos2) {
          ptr = internal[ptr].right;
          continue;
        }
        if (internal[ptr].splitPos1 > splitInfo->splitpos1)
          return ptr;
      }

      leafInfo = GetNumLeavesInSubtree(splitInfo, ptr, RIGHT);

      if (leafInfo.numLeavesInSubtree == 1) {
        ptr = leafInfo.leafNode + OFNODECARD;
        return ptr;
      }

      else {
        if (leafInfo.numLeavesUnderLeftChild == 0)
          ptr = internal[ptr].right;

        else if (leafInfo.numLeavesUnderLeftChild ==
                 leafInfo.numLeavesInSubtree)
          ptr = internal[ptr].left;

        else
          return ptr;
      }
    }
  }
}

void OFNode::ConstructOFSubtree(OFNode *subtreeNode,
                                struct OFnode_split_info *splitInfo,
                                int option) {
  int target, leftchild, rightchild;
  int rootptr;
  int ptr = 0;

  rootptr = 0; /* initially points to the root of the kdtree */

  rootptr = GetNextOFkdTreeNode(
      option, rootptr,
      splitInfo); /* get the root of the kdtree being constructed */

  /* pointing to an internal node */
  assert(rootptr < OFNODECARD);

  subtreeNode->internal[0] = internal[rootptr];
  subtreeNode->internal[0].parent = OFNODECARD;
  subtreeNode->numInternalNodes++;

  while (ptr < subtreeNode->numInternalNodes) {
    leftchild = (subtreeNode->internal[ptr]).left;
    if (leftchild < OFNODECARD)
      leftchild = GetNextOFkdTreeNode(option, leftchild, splitInfo);
    if (leftchild >= OFNODECARD) // left child of current node is a leaf node
    {
      assert((leftchild - OFNODECARD) < numLeafNodes);
      target = subtreeNode->numLeafNodes++;
      subtreeNode->leaf[target] = leaf[leftchild - OFNODECARD];
      subtreeNode->internal[ptr].left = target + OFNODECARD;
      subtreeNode->leaf[target].parent = ptr;
    } else // left child of current node is an internal node
    {
      assert(leftchild < numInternalNodes);
      target = subtreeNode->numInternalNodes++;
      subtreeNode->internal[target] = internal[leftchild];
      subtreeNode->internal[ptr].left = target;
      subtreeNode->internal[target].parent = ptr;
    }

    rightchild = (subtreeNode->internal[ptr]).right;
    if (rightchild < OFNODECARD)
      rightchild = GetNextOFkdTreeNode(option, rightchild, splitInfo);
    if (rightchild >= OFNODECARD) // right child of current node is a leaf node
    {
      assert((rightchild - OFNODECARD) < numLeafNodes);
      target = subtreeNode->numLeafNodes++;
      subtreeNode->leaf[target] = leaf[rightchild - OFNODECARD];
      subtreeNode->internal[ptr].right = target + OFNODECARD;
      subtreeNode->leaf[target].parent = ptr;
    } else // right child of current node is an internal node
    {
      assert(rightchild < numInternalNodes);
      target = subtreeNode->numInternalNodes++;
      subtreeNode->internal[target] = internal[rightchild];
      subtreeNode->internal[ptr].right = target;
      subtreeNode->internal[target].parent = ptr;
    }
    ptr++;
  }
}

void OFNode::SplitOFNodekdTree(OFNode **nn,
                               struct OFnode_split_info *splitInfo) {
  OFNode *replacementNode;

  (*nn) = new OFNode();
  replacementNode = new OFNode();
  (*nn)->level = level;
  replacementNode->level = level;

  assert(splitInfo->numRightChildren + splitInfo->numLeftChildren ==
         OFNODECARD);

  ConstructOFSubtree(replacementNode, splitInfo,
                     LEFT);                  /* construct left subtree */
  ConstructOFSubtree(*nn, splitInfo, RIGHT); /* construct right subtree */

  assert((replacementNode->numLeafNodes + (*nn)->numLeafNodes) == OFNODECARD);

  // printf("splitdimension = %d before split numleaves = %d left node numleaves
  // = %d rightnode numleaves = %d \n amount of overlap=%f",
  // splitInfo->splitdim, numLeafNodes, replacementNode->numLeafNodes,
  // (*nn)->numLeafNodes, (splitInfo->splitpos1-splitInfo->splitpos2));
  // getchar();

  CopyFrom(replacementNode);
  delete replacementNode;
}

void OFNode::GuaranteedAssessSplits(
    struct OFnode_split_info *split_choice_array, int numPossibleSplitDims,
    Rect *kdtreeLeafBRList, Rect *indexedRegion) {
  unsigned short splitDimension;
  float splitPosition1;
  float splitPosition2;
  float newsplitPosition1, newsplitPosition2;
  Rect OverlappingRegion, leafBR;
  struct lineseg leftBounds[OFNODECARD];
  struct lineseg rightBounds[OFNODECARD];
  int i, j, k, limit, leftptr, rightptr;
  int splittingLeaf;

  for (i = 0; i < numPossibleSplitDims; i++) {
    splitDimension = split_choice_array[i].splitdim;
    // splitDimension=i;
    // split_choice_array[i].splitdim=splitDimension;

    // project the leaf objects in this dimension, then sort those line
    // segments based on their left boundaries, and also based on their
    // right boundaries

    for (k = 0; k < OFNODECARD; k++) {
      leftBounds[k].leafNodeIndex = k;
      rightBounds[k].leafNodeIndex = k;
      leftBounds[k].leftbound = kdtreeLeafBRList[k].boundary[splitDimension];
      leftBounds[k].rightbound =
          kdtreeLeafBRList[k].boundary[splitDimension + NUMDIMS];
      rightBounds[k].leftbound = kdtreeLeafBRList[k].boundary[splitDimension];
      rightBounds[k].rightbound =
          kdtreeLeafBRList[k].boundary[splitDimension + NUMDIMS];
    }

    qsort(leftBounds, OFNODECARD, sizeof(struct lineseg),
          (int (*)(const void *, const void *))leftboundcmp2);

    qsort(rightBounds, OFNODECARD, sizeof(struct lineseg),
          (int (*)(const void *, const void *))rightboundcmp2);

    limit = (int)(OFNODECARD * MINOFNODEUTIL) + 1;

    split_choice_array[i].numLeftChildren = 0;
    split_choice_array[i].numRightChildren = 0;
    split_choice_array[i].numCascadingSplits = 0;
    splitPosition1 = -99999.99;
    splitPosition2 = 99999.99;

    leftptr = 0;
    rightptr = 0;

    for (k = 0; k < OFNODECARD; k++) {
      // select a left guy

      for (; leftptr < OFNODECARD; leftptr++)
        if (!IsPresent2(leftBounds[leftptr].leafNodeIndex,
                        split_choice_array[i].listOfRightChildren,
                        split_choice_array[i].numRightChildren))
          break;

      split_choice_array[i]
          .listOfLeftChildren[split_choice_array[i].numLeftChildren++] =
          leftBounds[leftptr].leafNodeIndex;
      splitPosition1 = MAX(splitPosition1, leftBounds[leftptr].rightbound);
      leftptr++;

      // select a right guy
      for (; rightptr < OFNODECARD; rightptr++)
        if (!IsPresent2(rightBounds[rightptr].leafNodeIndex,
                        split_choice_array[i].listOfLeftChildren,
                        split_choice_array[i].numLeftChildren))
          break;

      split_choice_array[i]
          .listOfRightChildren[split_choice_array[i].numRightChildren++] =
          rightBounds[rightptr].leafNodeIndex;
      splitPosition2 = MIN(splitPosition2, rightBounds[rightptr].leftbound);
      rightptr++;

      if (MIN(split_choice_array[i].numLeftChildren,
              split_choice_array[i].numRightChildren) >= limit)
        break;
    }

    // put the rest in the cascading splits section, distribute without
    // being concerned about utilization, already taken care of
    for (k = 0; k < OFNODECARD; k++) {
      if (!IsPresent2(k, split_choice_array[i].listOfLeftChildren,
                      split_choice_array[i].numLeftChildren) &&
          !IsPresent2(k, split_choice_array[i].listOfRightChildren,
                      split_choice_array[i].numRightChildren)) {
        split_choice_array[i].listOfSplittingChildren
            [split_choice_array[i].numCascadingSplits++] = k;
      }
    }
    assert((split_choice_array[i].numRightChildren +
            split_choice_array[i].numLeftChildren +
            split_choice_array[i].numCascadingSplits) == OFNODECARD);

    newsplitPosition1 = splitPosition1;
    newsplitPosition2 = splitPosition2;

    for (j = 0; j < split_choice_array[i].numCascadingSplits; j++) {
      splittingLeaf = split_choice_array[i].listOfSplittingChildren[j];
      leafBR = kdtreeLeafBRList[splittingLeaf];
      if ((leafBR.boundary[splitDimension + NUMDIMS] - splitPosition1) <
          (splitPosition2 -
           leafBR.boundary[splitDimension])) // the leaf should go to the left
                                             // partition
      {
        newsplitPosition1 =
            MAX(newsplitPosition1, leafBR.boundary[splitDimension + NUMDIMS]);
        split_choice_array[i]
            .listOfLeftChildren[split_choice_array[i].numLeftChildren] =
            splittingLeaf;
        split_choice_array[i].numLeftChildren++;
      } else if ((leafBR.boundary[splitDimension + NUMDIMS] - splitPosition1) >
                 (splitPosition2 - leafBR.boundary[splitDimension]))
      // the leaf should go to the right partition
      {
        newsplitPosition2 =
            MIN(newsplitPosition2, leafBR.boundary[splitDimension]);
        split_choice_array[i]
            .listOfRightChildren[split_choice_array[i].numRightChildren] =
            splittingLeaf;
        split_choice_array[i].numRightChildren++;
      }
      // resolve ties by putting into the partition that has less people
      else // the elongation needed is equal
      {
        if (split_choice_array[i].numLeftChildren >=
            split_choice_array[i].numRightChildren) {
          newsplitPosition2 =
              MIN(newsplitPosition2, leafBR.boundary[splitDimension]);
          split_choice_array[i]
              .listOfRightChildren[split_choice_array[i].numRightChildren] =
              splittingLeaf;
          split_choice_array[i].numRightChildren++;
        } else {
          newsplitPosition1 =
              MAX(newsplitPosition1, leafBR.boundary[splitDimension + NUMDIMS]);
          split_choice_array[i]
              .listOfLeftChildren[split_choice_array[i].numLeftChildren] =
              splittingLeaf;
          split_choice_array[i].numLeftChildren++;
        }
      }
    }

    assert((split_choice_array[i].numRightChildren +
            split_choice_array[i].numLeftChildren) == OFNODECARD);
    // OverlappingRegion=(*indexedRegion);
    // OverlappingRegion.boundary[splitDimension]=newsplitPosition2;
    // OverlappingRegion.boundary[splitDimension+NUMDIMS]=newsplitPosition1;
    // split_choice_array[i].amountOfOverlap=RTreeRectVolume(&OverlappingRegion);
    split_choice_array[i].amountOfOverlap =
        newsplitPosition1 - newsplitPosition2;
    split_choice_array[i].splitpos1 = newsplitPosition1;
    split_choice_array[i].splitpos2 = newsplitPosition2;
    assert((split_choice_array[i]).numRightChildren +
               (split_choice_array[i]).numLeftChildren ==
           OFNODECARD);
  }
}

void OFNode::SplitNode(OFNode **nn, struct split **newSplit,
                       Rect *indexedRegion, int cascading_splits_allowed,
                       ReinsertItem *reinsertList, int *numItems) {
  int i, h, z, j;
  int splitnode;
  Rect OverlappingRegion;
  unsigned int splitDimension;
  unsigned int splitdim;
  int leftrightsum;
  float splitpos;
  struct split_info split_by_node[OFNODECARD - 1];
  struct OFnode_split_info split_choice_array[NUMDIMS];
  float worst_case_util;
  unsigned int listOfPossibleSplitDims[NUMDIMS];
  int numPossibleSplitDims = 0;
  Rect kdtreeLeafBRList[OFNODECARD];

  assert(numInternalNodes == (OFNODECARD - 1)); // node is full, hence splitting

  // try cascading splits
  if (cascading_splits_allowed) {
    // check along which nodes split is possible along with cascading splits
    for (i = 0, j = 0; i < OFNODECARD - 1; i++) {
      if (internal[i].splitPos1 <= internal[i].splitPos2) {
        // internal[i] represents a non-overlapping partition
        split_by_node[j] =
            GetkdTreeSplitInfo(i); // if internal[i] is used as splitnode
        // new added
        splitnode = split_by_node[j].splitNode;
        splitdim = internal[splitnode].splitDim;
        splitpos = internal[splitnode].splitPos1;
        assert((splitpos >= indexedRegion->boundary[splitdim]) &&
               (splitpos <= indexedRegion->boundary[splitdim + NUMDIMS]));
        // end new added
        j++;
      }
    }

    splitnode = -1; // initialize splitnode
    if (j > 0) {
      qsort(split_by_node, j, sizeof(struct split_info),
            (int (*)(const void *, const void *))splitcmp);
      for (i = 0; i < j; i++) {
        // if (split_by_node[i].numCascadingSplits > MAXCASCADINGSPLITS) break;
        if (split_by_node[i].numCascadingSplits > OFNODECARD)
          break;
        // worst_case_util=MIN(((float)(split_by_node[i].numCascadingSplits +
        // split_by_node[i].numLeftChildren)/(OFNODECARD)),((float)(split_by_node[i].numCascadingSplits
        // + split_by_node[i].numRightChildren)/(OFNODECARD)));
        worst_case_util =
            MIN(((float)(split_by_node[i].numLeftChildren) / (OFNODECARD)),
                ((float)(split_by_node[i].numRightChildren) / (OFNODECARD)));
        if (worst_case_util > MINOFNODEUTIL) {

          splitnode = split_by_node[i].splitNode;
          break;
        }
      }

      if (splitnode >= 0) {
        // this function will analyze whether you should do
        // OF style splitting or not
        if (AnalyzeSplit(splitnode) == SHOULD_SPLIT) {
#ifdef HTREE_DEBUG
#endif
          (*newSplit) = (struct split *)malloc(sizeof(struct split));
          (*newSplit)->splitDim = internal[splitnode].splitDim;
          (*newSplit)->splitPos = internal[splitnode].splitPos1;
          (*newSplit)->splitPos2 = internal[splitnode].splitPos2;
          assert((*newSplit)->splitPos == (*newSplit)->splitPos2);

          SplitkdTree(nn, splitnode, split_by_node[i].numCascadingSplits,
                      reinsertList, numItems);
          assert((numLeafNodes + (*nn)->numLeafNodes -
                  split_by_node[i].numCascadingSplits) <= OFNODECARD);
#ifdef HTREE_DEBUG
          printf("OFNode split (***** Overlap Free *****): splitnode =%d split "
                 "dimensions = %d split position = %f ",
                 splitnode, internal[splitnode].splitDim,
                 internal[splitnode].splitPos1);
          printf("num cascading splits = %d underful nodes eliminated = %d \n",
                 split_by_node[i].numCascadingSplits,
                 (OFNODECARD - (numLeafNodes + (*nn)->numLeafNodes -
                                split_by_node[i].numCascadingSplits)));
#endif
          return;
        }
      }
    }
  }
  // if cascading splits not possible i.e. all internal nodes in the
  // OFNode represent overlapping partitions or not allowed,
  // go for MONode-style node split

  GetkdtreeLeafBRs(0, indexedRegion, kdtreeLeafBRList);
  // what happens if we split by the root
  split_choice_array[0] = GetOFNodeSplitInfo(0);
  // check whether possible to split by root (i.e. preserving utilization
  // constraints)
  assert(split_choice_array[0].numCascadingSplits == 0);
  assert((split_choice_array[0]).numRightChildren +
             (split_choice_array[0]).numLeftChildren ==
         OFNODECARD);

  worst_case_util =
      MIN(((float)(split_choice_array[0].numLeftChildren) / (OFNODECARD)),
          ((float)(split_choice_array[0].numRightChildren) / (OFNODECARD)));
  if (split_choice_array[0].numLeftChildren > 1 &&
      split_choice_array[0].numRightChildren > 1 &&
      worst_case_util > MINOFNODEUTIL) // splitting by the root of the kdtree
  {
    i = 0;
    split_choice_array[0].splitpos1 = internal[0].splitPos1;
    split_choice_array[0].splitpos2 = internal[0].splitPos2;
    splitDimension = split_choice_array[0].splitdim;
    split_choice_array[0].amountOfOverlap =
        split_choice_array[0].splitpos1 - split_choice_array[0].splitpos2;
  } else // not splitting by the root
  {
    // consider only the dimensions with which this node has been split
    numPossibleSplitDims = 0;
    for (h = 0; h < OFNODECARD - 1; h++) {
      if (!IsPresent2(internal[h].splitDim, listOfPossibleSplitDims,
                      numPossibleSplitDims)) {
        listOfPossibleSplitDims[numPossibleSplitDims] = internal[h].splitDim;
        split_choice_array[numPossibleSplitDims].splitdim =
            internal[h].splitDim;
        numPossibleSplitDims++;
      }
    }
    GuaranteedAssessSplits(split_choice_array, numPossibleSplitDims,
                           kdtreeLeafBRList, indexedRegion);
    qsort(split_choice_array, numPossibleSplitDims,
          sizeof(struct OFnode_split_info),
          (int (*)(const void *, const void *))overlapcmp2);

    for (i = 0; i < numPossibleSplitDims; i++) {
      if (split_choice_array[i].numLeftChildren > 1 &&
          split_choice_array[i].numRightChildren > 1) {
        worst_case_util = MIN(
            ((float)(split_choice_array[i].numLeftChildren) / (OFNODECARD)),
            ((float)(split_choice_array[i].numRightChildren) / (OFNODECARD)));
        if (worst_case_util > MINOFNODEUTIL) {
          break;
        }
      }
    }
  }
  (*newSplit) = (struct split *)malloc(sizeof(struct split));
  (*newSplit)->splitDim = split_choice_array[i].splitdim;
  (*newSplit)->splitPos = split_choice_array[i].splitpos1;
  (*newSplit)->splitPos2 = split_choice_array[i].splitpos2;
  SplitOFNodekdTree(nn, &(split_choice_array[i]));
  assert((numLeafNodes + (*nn)->numLeafNodes) == OFNODECARD);
#ifdef HTREE_DEBUG
  printf("OFNode split (***** Minimal Overlap *****): split dimension = %d "
         "split position left = %f split position right = %f\n",
         split_choice_array[i].splitdim, split_choice_array[i].splitpos1,
         split_choice_array[i].splitpos2);
#endif
  return;
}

void OFNode::CreatekdTree(Node *leftDataNode, Node *rightDataNode,
                          struct split *newSplit) {
  assert(leftDataNode->type == DATANODE);
  assert(rightDataNode->type == DATANODE);
  numInternalNodes = 1;
  numLeafNodes = 2;
  internal[0].splitDim = newSplit->splitDim;
  internal[0].splitPos1 = newSplit->splitPos;
  internal[0].splitPos2 = newSplit->splitPos;
  internal[0].left = OFNODECARD;
  internal[0].right = OFNODECARD + 1;
  internal[0].parent = OFNODECARD; // means junk, no parent
  leaf[0].parent = 0;
  leaf[0].child = leftDataNode;
  leaf[1].parent = 0;
  leaf[1].child = rightDataNode;
}

// ------------- start of OF-node style split functions --------------
// ------------- start of OF-node style split functions --------------
// ------------- start of OF-node style split functions --------------

// returns SHOULD_SPLIT or SHOULD_NOT_SPLIT depending on 2 criteria
// it's ok to go for a particular cascading split which satisfies node
// utilization if: (1) no (cascading) data node split cause both
// partitions to be underfull and (2) not too many reinsertions (<MAX)
int OFNode::AnalyzeSplit(int splitnode) {
  int i, num_reinsertions;
  int total_reinsertions = 0;
  DataNode *datanode;
  int leaf_index;
  int status;
  // find the list of datanodes that will be split due to this "OFnode-style"
  // split
  unsigned int splittingLeaves[OFNODECARD];
  int splittingLeafCount = 0;
  // splittingLeaves contains all the leaf nodes that are splitting due
  // to this split.
  // the number of such leaves is stored in splittingLeafCount
  assert(internal[splitnode].splitPos1 == internal[splitnode].splitPos2);
  GetListOfSplittingDataNodes(0, splitnode, internal[splitnode].splitDim,
                              internal[splitnode].splitPos1, splittingLeaves,
                              &splittingLeafCount);

  // for each splitting datanode,
  // find out the leftcount and rightcount
  for (i = 0; i < splittingLeafCount; i++) {
    leaf_index = splittingLeaves[i];
    datanode = (DataNode *)leaf[leaf_index - OFNODECARD].child;
    assert(datanode != NULL);
    status = datanode->ComputeNumReinsertions(internal[splitnode].splitDim,
                                              internal[splitnode].splitPos1,
                                              &num_reinsertions);
    if (status == BOTH_PARTITIONS_UNDERFULL) {
#ifdef HTREE_DEBUG
      printf("OF-split will cause both partitions underfull... so going for MO "
             "\n");
#endif
      return SHOULD_NOT_SPLIT;
    } else
      total_reinsertions += num_reinsertions;
    if (total_reinsertions > MAX_REINSERTIONS_ALLOWED) {
#ifdef HTREE_DEBUG
      printf("OF-split will cause too many reinsertions (%d)... so going for "
             "MO \n",
             total_reinsertions);
#endif
      return SHOULD_NOT_SPLIT;
    }
  }
  return SHOULD_SPLIT;
}

unsigned int OFNode::GetSibling(unsigned int index, int option) {
  unsigned int parent;

  if (option == LEAF) {
    parent = leaf[index].parent;
    index += OFNODECARD;
  } else
    parent = internal[index].parent;

  if (internal[parent].left == index)
    return internal[parent].right;
  else {
    assert(internal[parent].right == index);
    return internal[parent].left;
  }
}

int OFNode::WhichChildOfParent(unsigned int index, int option) {
  unsigned int parent;

  if (option == LEAF) {
    parent = leaf[index].parent;
    index += OFNODECARD;
  } else
    parent = internal[index].parent;

  if (parent >= OFNODECARD || parent < 0)
    return -1;

  if (internal[parent].left == index)
    return LEFT;
  else {
    assert(internal[parent].right == index);
    return RIGHT;
  }
}

int OFNode::RemoveLeaf(int i) {
  unsigned int parent, grandparent, parent2, parent3, sibling, rightchild,
      leftchild;
  int jj, kk;

  parent = leaf[i].parent;
  sibling = GetSibling(i, LEAF);
  if (parent != 0) {
    grandparent = internal[parent].parent;

    if (WhichChildOfParent(parent, INTERNAL) == LEFT)
      internal[grandparent].left = sibling;
    else
      internal[grandparent].right = sibling;

    if (sibling >= OFNODECARD)
      leaf[sibling - OFNODECARD].parent = grandparent;
    else
      internal[sibling].parent = grandparent;
  } else {
    if (sibling >= OFNODECARD)
      return 0; // cannot remove leaf becuase there are only 2 leaves in the
                // node
    else {
      assert(sibling == 1); // since sibling is the only son of the root which
                            // is an internal node
      internal[sibling].parent =
          OFNODECARD; // else make the sibling the new root
    }
  }
  // the i th slot among leaf nodes is free, so do compaction
  for (jj = i + 1; jj < numLeafNodes; jj++) {
    leaf[jj - 1] = leaf[jj];
    parent2 = leaf[jj].parent;
    if (WhichChildOfParent(jj, LEAF) == LEFT)
      internal[parent2].left = (jj + OFNODECARD - 1);
    else
      internal[parent2].right = (jj + OFNODECARD - 1);
  }

  // the parent slot among internal nodes is free, so do compaction
  for (kk = parent + 1; kk < numInternalNodes; kk++) {
    internal[kk - 1] = internal[kk];
    parent3 = internal[kk].parent;

    if (parent3 < OFNODECARD) {
      if (WhichChildOfParent(kk, INTERNAL) == LEFT)
        internal[parent3].left = kk - 1;
      else
        internal[parent3].right = kk - 1;
    }

    leftchild = internal[kk - 1].left;
    rightchild = internal[kk - 1].right;

    if (leftchild >= OFNODECARD)
      leaf[leftchild - OFNODECARD].parent = kk - 1;
    else
      internal[leftchild].parent = kk - 1;

    if (rightchild >= OFNODECARD)
      leaf[rightchild - OFNODECARD].parent = kk - 1;
    else
      internal[rightchild].parent = kk - 1;
  }
  numInternalNodes--;
  numLeafNodes--;
  return 1; // removeleaf successful
}

// the following set of functions are to split a OF node
// the OF node is split in a OF manner, this may cause
// cascading splits. however, the cascade is retricted to
// just a one level since there is only one level below the
// level of OF nodes. we choose the split with least number
// of cascading splits which satisfies the min. utilization

// return the number of leaves below the node pointed by index
int OFNode::GetNumLeavesInSubTree(int index) {
  if (index < OFNODECARD) // internal node
    return (GetNumLeavesInSubTree(internal[index].left) +
            GetNumLeavesInSubTree(internal[index].right));
  else
    return 1; // leaf node
}

// internal[stopnode] is the node being used to split, so how many cascading
// splits, how many left children, how may right children etc.

void OFNode::GetSplitInfo(unsigned int subtreeroot, unsigned int stopnode,
                          unsigned short splitdim, float splitpos,
                          struct split_info *newSplitInfo) {
  if (subtreeroot >= OFNODECARD)
    newSplitInfo->numCascadingSplits++;
  else // internal node
  {
    if (internal[subtreeroot].splitDim != splitdim) {
      GetSplitInfo(internal[subtreeroot].left, stopnode, splitdim, splitpos,
                   newSplitInfo);
      GetSplitInfo(internal[subtreeroot].right, stopnode, splitdim, splitpos,
                   newSplitInfo);
    } else // n->rest.ofnode.internal[subtreeroot].splitDim == splitdim
    {
      if (internal[subtreeroot].splitPos1 <= splitpos)
        newSplitInfo->numLeftChildren +=
            GetNumLeavesInSubTree(internal[subtreeroot].left);
      else
        GetSplitInfo(internal[subtreeroot].left, stopnode, splitdim, splitpos,
                     newSplitInfo);

      if (internal[subtreeroot].splitPos2 >= splitpos)
        newSplitInfo->numRightChildren +=
            GetNumLeavesInSubTree(internal[subtreeroot].right);
      else
        GetSplitInfo(internal[subtreeroot].right, stopnode, splitdim, splitpos,
                     newSplitInfo);
    }
  }
}

void OFNode::GetListOfSplittingDataNodes(unsigned int subtreeroot,
                                         unsigned int stopnode,
                                         unsigned short splitdim,
                                         float splitpos, unsigned int *list,
                                         int *listcount) {
  if (subtreeroot >= OFNODECARD) {
    list[*listcount] = subtreeroot;
    (*listcount)++;
  } else // internal node
  {
    if (subtreeroot != stopnode) {
      if (internal[subtreeroot].splitDim != splitdim) {
        GetListOfSplittingDataNodes(internal[subtreeroot].left, stopnode,
                                    splitdim, splitpos, list, listcount);
        GetListOfSplittingDataNodes(internal[subtreeroot].right, stopnode,
                                    splitdim, splitpos, list, listcount);
      } else // n->rest.ofnode.internal[subtreeroot].splitDim == splitdim
      {
        if (internal[subtreeroot].splitPos2 < splitpos) {
          GetListOfSplittingDataNodes(internal[subtreeroot].right, stopnode,
                                      splitdim, splitpos, list, listcount);
        }
        if (internal[subtreeroot].splitPos1 > splitpos) {
          GetListOfSplittingDataNodes(internal[subtreeroot].left, stopnode,
                                      splitdim, splitpos, list, listcount);
        }
      }
    }
  }
}

// return split information i.e. number of cascading splits,
//   utilization etc. if internal[index] is used as the split
//   hyperplane, so index is the stopnode

struct split_info OFNode::GetkdTreeSplitInfo(int index) {
  struct split_info newSplitInfo;
  unsigned short splitdim = internal[index].splitDim;
  assert(internal[index].splitPos1 == internal[index].splitPos2);
  float splitpos = internal[index].splitPos1;
  // initialize
  newSplitInfo.splitNode = index;
  newSplitInfo.numCascadingSplits = 0;
  newSplitInfo.numLeftChildren = 0;
  newSplitInfo.numRightChildren = 0;
  GetSplitInfo(0, index, splitdim, splitpos, &newSplitInfo);
  // newSplitInfo.numLeftChildren += GetNumLeavesInSubTree(n,
  // n->rest.ofnode.internal[index].left); newSplitInfo.numRightChildren +=
  // GetNumLeavesInSubTree(n, n->rest.ofnode.internal[index].right);
  assert((newSplitInfo.numRightChildren + newSplitInfo.numLeftChildren +
          newSplitInfo.numCascadingSplits) == OFNODECARD);

  return newSplitInfo;
}

int OFNode::GetNextNode(int splitnode, int option, int currnodeptr) {
  int ptr = currnodeptr;
  assert(ptr < (OFNODECARD - 1)); // pointing to an internal node

  for (;;) {
    if (ptr >= OFNODECARD)
      return ptr; // pointing to a leaf node
    if (internal[ptr].splitDim == internal[splitnode].splitDim) {
      if (ptr == splitnode) {
        if (option == LEFT)
          ptr = internal[ptr].left;
        else
          ptr = internal[ptr].right;
      } else {
        if (option == LEFT &&
            internal[ptr].splitPos2 >= internal[splitnode].splitPos1)
          ptr = internal[ptr].left;
        else if (option == RIGHT &&
                 internal[ptr].splitPos1 <= internal[splitnode].splitPos2)
          ptr = internal[ptr].right;
        else
          return ptr;
      }
    } else
      return ptr;
  }
}

void OFNode::ConstructSubtree(OFNode *subtreeNode, int splitnode, int option,
                              DataNode **deleted_child_nodes,
                              int *num_deleted_child_nodes,
                              ReinsertItem *reinsertion_list,
                              int *num_points_in_list) {
  int target, leftchild, rightchild;
  int rootptr;
  int status;
  struct kdTreeInternalNode intNode;
  int ptr = 0;
  unsigned int splittingLeaves[OFNODECARD];
  int splittingLeafCount = 0;
  assert(subtreeNode->type == OFNODE);

  // splittingLeaves contains all the leaf nodes that are splitting due to this
  // split.
  //   The number of such leaves is stored in splittingLeafCount

  assert(internal[splitnode].splitPos1 == internal[splitnode].splitPos2);
  GetListOfSplittingDataNodes(0, splitnode, internal[splitnode].splitDim,
                              internal[splitnode].splitPos1, splittingLeaves,
                              &splittingLeafCount);

  rootptr = 0; // initially points to the root of the kdtree

  rootptr =
      GetNextNode(splitnode, option,
                  rootptr); // get the root of the kdtree being constructed

  assert(rootptr < OFNODECARD); // pointing to an internal node
  subtreeNode->internal[0] = internal[rootptr];
  subtreeNode->internal[0].parent = OFNODECARD;
  subtreeNode->numInternalNodes++;

  while (ptr < subtreeNode->NumInternalNodes()) {
    leftchild = (subtreeNode->Internal(ptr)).left;
    if (leftchild < OFNODECARD)
      leftchild = GetNextNode(splitnode, option, leftchild);
    if (leftchild >= OFNODECARD) // left child of current node is a leaf node
    {
      assert((leftchild - OFNODECARD) < numLeafNodes);
      target = subtreeNode->numLeafNodes++;

      if (IsASplittingLeaf(leftchild, splittingLeaves, splittingLeafCount) &&
          (option == RIGHT)) {
        // if leaf[leftchild-OFNODECARD].child is split at
        // internal[splitnode].splitDim, internal[splitnode].splitPos2
        // compute reinsertions if required
        // returns NEITHER_PARTITION_UNDERFULL if both partitions
        // satisfy node util,  RIGHT_PARTITION_UNDERFULL if only left
        // partition satisfy node util, LEFT_PARTITION_UNDERFULL if
        // only right one satisfies node util, should not
        // return BOTH_PARTITIONS_UNDERFULL
        // in the latter 2 cases (RIGHT_PARTITION_UNDERFULL and
        // LEFT_PARTITION_UNDERFULL, inserts reinsertions in
        // reinsertion_list
        status =
            ((DataNode *)(leaf[leftchild - OFNODECARD].child))
                ->ComputeReinsertions(internal[splitnode].splitDim,
                                      internal[splitnode].splitPos2,
                                      reinsertion_list, num_points_in_list);

        if (status == NEITHER_PARTITION_UNDERFULL) {
          subtreeNode->leaf[target].child = new DataNode();
          ((DataNode *)(leaf[leftchild - OFNODECARD].child))
              ->DistributeDataItems(
                  (DataNode *)(subtreeNode->leaf[target].child),
                  internal[splitnode].splitDim, internal[splitnode].splitPos2);
        } else if (status == RIGHT_PARTITION_UNDERFULL)
          subtreeNode->leaf[target].child = NULL;
        else if (status == LEFT_PARTITION_UNDERFULL) {
          subtreeNode->leaf[target].child = leaf[leftchild - OFNODECARD].child;

          deleted_child_nodes[*num_deleted_child_nodes] =
              (DataNode *)leaf[leftchild - OFNODECARD].child;
          (*num_deleted_child_nodes)++;
        } else
          assert(FALSE); // both partitions underfull, cannot happen

        subtreeNode->internal[ptr].left = target + OFNODECARD;
        subtreeNode->leaf[target].parent = ptr;
      } else {
        subtreeNode->leaf[target] = leaf[leftchild - OFNODECARD];
        subtreeNode->internal[ptr].left = target + OFNODECARD;
        subtreeNode->leaf[target].parent = ptr;
      }
    } else // left child of current node is an internal node
    {
      assert(leftchild < numInternalNodes);
      target = subtreeNode->numInternalNodes++;
      subtreeNode->internal[target] = internal[leftchild];
      subtreeNode->internal[ptr].left = target;
      subtreeNode->internal[target].parent = ptr;
    }

    rightchild = (subtreeNode->internal[ptr]).right;
    if (rightchild < OFNODECARD)
      rightchild = GetNextNode(splitnode, option, rightchild);
    if (rightchild >= OFNODECARD) // right child of current node is a leaf node
    {
      assert((rightchild - OFNODECARD) < numLeafNodes);
      target = subtreeNode->numLeafNodes++;

      if (IsASplittingLeaf(rightchild, splittingLeaves, splittingLeafCount) &&
          (option == RIGHT)) {

        status =
            ((DataNode *)(leaf[rightchild - OFNODECARD].child))
                ->ComputeReinsertions(internal[splitnode].splitDim,
                                      internal[splitnode].splitPos2,
                                      reinsertion_list, num_points_in_list);

        if (status == NEITHER_PARTITION_UNDERFULL) {
          subtreeNode->leaf[target].child = new DataNode();
          ((DataNode *)(leaf[rightchild - OFNODECARD].child))
              ->DistributeDataItems(
                  (DataNode *)(subtreeNode->leaf[target].child),
                  internal[splitnode].splitDim, internal[splitnode].splitPos2);
        } else if (status == RIGHT_PARTITION_UNDERFULL)
          subtreeNode->leaf[target].child = NULL;
        else if (status == LEFT_PARTITION_UNDERFULL) {
          subtreeNode->leaf[target].child = leaf[rightchild - OFNODECARD].child;
          deleted_child_nodes[*num_deleted_child_nodes] =
              (DataNode *)leaf[leftchild - OFNODECARD].child;
          (*num_deleted_child_nodes)++;
        } else
          assert(FALSE); // both partitions underfull, cannot happen

        subtreeNode->internal[ptr].right = target + OFNODECARD;
        subtreeNode->leaf[target].parent = ptr;
      } else {
        subtreeNode->leaf[target] = leaf[rightchild - OFNODECARD];
        subtreeNode->internal[ptr].right = target + OFNODECARD;
        subtreeNode->leaf[target].parent = ptr;
      }
    } else // right child of current node is an internal node
    {
      assert(rightchild < numInternalNodes);
      target = subtreeNode->numInternalNodes++;
      subtreeNode->internal[target] = internal[rightchild];
      subtreeNode->internal[ptr].right = target;
      subtreeNode->internal[target].parent = ptr;
    }
    ptr++;
  }
}

int OFNode::EliminateEmptyNodes(DataNode **deleted_child_nodes,
                                int num_deleted_child_nodes) {
  int i;

  for (i = 0; i < numLeafNodes; i++) {
    if (leaf[i].child == NULL) { // node is empty
      if (RemoveLeaf(i))
        i--;
      else {
        perror("Problem in splitting OFNode: kdtree too small!!\n");
        exit(0);
      }
    } else { // node is not empty — check if it is in the deleted list
      if (deleted_child_nodes != NULL) {
        if (IsPresent((DataNode *)leaf[i].child, deleted_child_nodes,
                      num_deleted_child_nodes)) {
          leaf[i].child = NULL;
          if (RemoveLeaf(i))
            i--;
          else {
            perror("Problem in splitting OFNode: kdtree too small!!\n");
            exit(0);
          }
        }
      }
    }
  }
  return numLeafNodes; /* number of remaining leaf nodes after elimination */
}

void OFNode::SplitkdTree(OFNode **nn, int splitnode, int num_cascading_splits,
                         ReinsertItem *reinsertList, int *numItems) {
  DataNode *deleted_child_nodes[OFNODECARD];
  int num_deleted_child_nodes = 0;

  OFNode *replacementNode;
  (*nn) = new OFNode();
  replacementNode = new OFNode();
  (*nn)->level = level;
  replacementNode->level = level;
  DataNode *datanode;
  ConstructSubtree(replacementNode, splitnode, LEFT); // construct left subtree
  ConstructSubtree(*nn, splitnode, RIGHT, deleted_child_nodes,
                   &num_deleted_child_nodes, reinsertList,
                   numItems); // construct right subtree
  replacementNode->EliminateEmptyNodes(deleted_child_nodes,
                                       num_deleted_child_nodes);
  (*nn)->EliminateEmptyNodes();
  CopyFrom(replacementNode);
  delete replacementNode;
}

void OFNode::CopyFrom(OFNode *n) {
  int i;
  numInternalNodes = n->numInternalNodes;
  numLeafNodes = n->numLeafNodes;
  for (i = 0; i < OFNODECARD - 1; i++)
    internal[i] = n->internal[i];
  for (i = 0; i < OFNODECARD; i++)
    leaf[i] = n->leaf[i];
}

// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES *******************
// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES *******************
// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES *******************

// fill in the children list i.e. list of children of an OF node
// along with their BRs
void OFNode::FillChildList(int currRoot, MultiPointQuery *MPQ,
                           Rect *indexedRegion, PQ *queue, int dist_func) {
  int i;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  float distance_from_centroid, radial_dist, distance;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  intNode = &(internal[currRoot]);

  leftIndexedRegion = *indexedRegion;
  leftIndexedRegion.High(intNode->splitDim, intNode->splitPos1);

  if (intNode->left >= OFNODECARD) {
    if (leaf[intNode->left - OFNODECARD].elsvalid) {
      liveRegion.DecodeLiveSpace(leaf[intNode->left - OFNODECARD].elsarray,
                                 &leftIndexedRegion);
      distance = MPQ->Distance(&liveRegion, dist_func);
      queue->push(PQEntry((leaf[intNode->left - OFNODECARD]).child, NODE,
                          leftIndexedRegion, distance,
                          (leaf[intNode->left - OFNODECARD]).count,
                          leaf[intNode->left - OFNODECARD].elsarray));
    } else {
      distance = MPQ->Distance(&leftIndexedRegion, dist_func);
      queue->push(PQEntry((leaf[intNode->left - OFNODECARD]).child, NODE,
                          leftIndexedRegion, distance,
                          (leaf[intNode->left - OFNODECARD]).count, NULL));
    }
  } else
    FillChildList(intNode->left, MPQ, &leftIndexedRegion, queue, dist_func);

  rightIndexedRegion = *indexedRegion;
  rightIndexedRegion.Low(intNode->splitDim, intNode->splitPos2);

  if (intNode->right >= OFNODECARD) {
    if (leaf[intNode->right - OFNODECARD].elsvalid) {
      liveRegion.DecodeLiveSpace(leaf[intNode->right - OFNODECARD].elsarray,
                                 &rightIndexedRegion);
      distance = MPQ->Distance(&liveRegion, dist_func);
      queue->push(PQEntry((leaf[intNode->right - OFNODECARD]).child, NODE,
                          rightIndexedRegion, distance,
                          (leaf[intNode->right - OFNODECARD]).count,
                          leaf[intNode->right - OFNODECARD].elsarray));
    } else {
      distance = MPQ->Distance(&rightIndexedRegion, dist_func);
      queue->push(PQEntry((leaf[intNode->right - OFNODECARD]).child, NODE,
                          rightIndexedRegion, distance,
                          (leaf[intNode->right - OFNODECARD]).count, NULL));
    }
  } else
    FillChildList(intNode->right, MPQ, &rightIndexedRegion, queue, dist_func);
}

// fill in the children list i.e. list of children of an OF node
// along with their BRs
void OFNode::FillChildList(int currRoot, MultiPointQuery *MPQ,
                           Rect *indexedRegion, PQ *queue,
                           float (*User_Distance)(Point *, Point *)) {
  int i;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  float distance_from_centroid, radial_dist, distance;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  intNode = &(internal[currRoot]);

  leftIndexedRegion = *indexedRegion;
  leftIndexedRegion.High(intNode->splitDim, intNode->splitPos1);

  if (intNode->left >= OFNODECARD) {
    if (leaf[intNode->left - OFNODECARD].elsvalid) {
      liveRegion.DecodeLiveSpace(leaf[intNode->left - OFNODECARD].elsarray,
                                 &leftIndexedRegion);
      distance = MPQ->Distance(&liveRegion, User_Distance);
      queue->push(PQEntry((leaf[intNode->left - OFNODECARD]).child, NODE,
                          leftIndexedRegion, distance,
                          (leaf[intNode->left - OFNODECARD]).count,
                          leaf[intNode->left - OFNODECARD].elsarray));
    } else {
      distance = MPQ->Distance(&leftIndexedRegion, User_Distance);
      queue->push(PQEntry((leaf[intNode->left - OFNODECARD]).child, NODE,
                          leftIndexedRegion, distance,
                          (leaf[intNode->left - OFNODECARD]).count, NULL));
    }
  } else
    FillChildList(intNode->left, MPQ, &leftIndexedRegion, queue, User_Distance);

  rightIndexedRegion = *indexedRegion;
  rightIndexedRegion.Low(intNode->splitDim, intNode->splitPos2);

  if (intNode->right >= OFNODECARD) {
    if (leaf[intNode->right - OFNODECARD].elsvalid) {
      liveRegion.DecodeLiveSpace(leaf[intNode->right - OFNODECARD].elsarray,
                                 &rightIndexedRegion);
      distance = MPQ->Distance(&liveRegion, User_Distance);
      queue->push(PQEntry((leaf[intNode->right - OFNODECARD]).child, NODE,
                          rightIndexedRegion, distance,
                          (leaf[intNode->right - OFNODECARD]).count,
                          leaf[intNode->right - OFNODECARD].elsarray));
    } else {
      distance = MPQ->Distance(&rightIndexedRegion, User_Distance);
      queue->push(PQEntry((leaf[intNode->right - OFNODECARD]).child, NODE,
                          rightIndexedRegion, distance,
                          (leaf[intNode->right - OFNODECARD]).count, NULL));
    }
  } else
    FillChildList(intNode->right, MPQ, &rightIndexedRegion, queue,
                  User_Distance);
}

// this function will use the centroid info if it is valid
// i.e. USE_CENTROID is set and dist_func == DIST_METRIC (should be lb)
// has already been checked
/* for distance based queries, using inbuilt-dist functions */
void OFNode::FillVisitList(MultiPointQuery *mpq, int currRoot,
                           struct region *visitlist, int *visitcount,
                           Rect *indexedRegion, float max_distance,
                           int dist_func) {
  int i, kk, ii;
  float dist_array[NUMDIMS];
  unsigned int sum;
  float distance;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  float distance_from_centroid, radial_dist;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  if (numLeafNodes <= 0) {
    *visitcount = 0;
    return;
  }
  intNode = &(internal[currRoot]);

  leftIndexedRegion = *indexedRegion;
  leftIndexedRegion.High(intNode->splitDim, intNode->splitPos1);

  // query overlaps with left partition
  if (leftIndexedRegion.Overlap(mpq, max_distance, dist_func)) {
    if (intNode->left >= OFNODECARD) {
      if (leaf[intNode->left - OFNODECARD].elsvalid) {
        liveRegion.DecodeLiveSpace(leaf[intNode->left - OFNODECARD].elsarray,
                                   &leftIndexedRegion);

        if (liveRegion.Overlap(mpq, max_distance, dist_func)) {
          if (USE_CENTROID) {
            centroid = liveRegion.Center();
            distance_from_centroid = mpq->Distance(&centroid, DIST_METRIC);
            radial_dist = distance_from_centroid -
                          leaf[intNode->left - OFNODECARD].radius;
            if (radial_dist <= max_distance) {
              visitlist[*visitcount].index = (intNode->left) - OFNODECARD;
              visitlist[*visitcount].indexedRegion = leftIndexedRegion;
              (*visitcount)++;
            }
          } else {
            visitlist[*visitcount].index = (intNode->left) - OFNODECARD;
            visitlist[*visitcount].indexedRegion = leftIndexedRegion;
            (*visitcount)++;
          }
        }
      } else {
        visitlist[*visitcount].index = (intNode->left) - OFNODECARD;
        visitlist[*visitcount].indexedRegion = leftIndexedRegion;
        (*visitcount)++;
      }
    } else {
      FillVisitList(mpq, intNode->left, visitlist, visitcount,
                    &leftIndexedRegion, max_distance, dist_func);
    }
  }

  rightIndexedRegion = *indexedRegion;
  rightIndexedRegion.Low(intNode->splitDim, intNode->splitPos2);

  // query overlaps with right partition
  if (rightIndexedRegion.Overlap(mpq, max_distance, dist_func)) {
    if (intNode->right >= OFNODECARD) {
      if (leaf[intNode->right - OFNODECARD].elsvalid) {
        liveRegion.DecodeLiveSpace(leaf[intNode->right - OFNODECARD].elsarray,
                                   &rightIndexedRegion);
        if (liveRegion.Overlap(mpq, max_distance, dist_func)) {
          if (USE_CENTROID) {
            centroid = liveRegion.Center();
            distance_from_centroid = mpq->Distance(&centroid, DIST_METRIC);
            radial_dist = distance_from_centroid -
                          leaf[intNode->right - OFNODECARD].radius;
            if (radial_dist <= max_distance) {
              visitlist[*visitcount].index = (intNode->right) - OFNODECARD;
              visitlist[*visitcount].indexedRegion = rightIndexedRegion;
              (*visitcount)++;
            }
          } else {
            visitlist[*visitcount].index = (intNode->right) - OFNODECARD;
            visitlist[*visitcount].indexedRegion = rightIndexedRegion;
            (*visitcount)++;
          }
        }
      } else {
        visitlist[*visitcount].index = (intNode->right) - OFNODECARD;
        visitlist[*visitcount].indexedRegion = rightIndexedRegion;
        (*visitcount)++;
      }
    } else {
      FillVisitList(mpq, intNode->right, visitlist, visitcount,
                    &rightIndexedRegion, max_distance, dist_func);
    }
  }
}

/* for distance based queries, using user-specified dist func */
void OFNode::FillVisitList(MultiPointQuery *mpq, int currRoot,
                           struct region *visitlist, int *visitcount,
                           Rect *indexedRegion, float max_distance,
                           float (*User_Distance)(Point *, Point *)) {
  int i, kk, ii;
  float dist_array[NUMDIMS];
  unsigned int sum;
  float distance;
  struct kdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  assert(currRoot < (OFNODECARD - 1)); // must be an internal kdtree node

  if (numLeafNodes <= 0) {
    *visitcount = 0;
    return;
  }
  intNode = &(internal[currRoot]);

  leftIndexedRegion = *indexedRegion;
  leftIndexedRegion.High(intNode->splitDim, intNode->splitPos1);

  // query overlaps with left partition
  if (leftIndexedRegion.Overlap(mpq, max_distance, User_Distance)) {
    if (intNode->left >= OFNODECARD) {
      if (leaf[intNode->left - OFNODECARD].elsvalid) {
        liveRegion.DecodeLiveSpace(leaf[intNode->left - OFNODECARD].elsarray,
                                   &leftIndexedRegion);
        if (liveRegion.Overlap(mpq, max_distance, User_Distance)) {
          visitlist[*visitcount].index = (intNode->left) - OFNODECARD;
          visitlist[*visitcount].indexedRegion = leftIndexedRegion;
          (*visitcount)++;
        }
      } else {
        visitlist[*visitcount].index = (intNode->left) - OFNODECARD;
        visitlist[*visitcount].indexedRegion = leftIndexedRegion;
        (*visitcount)++;
      }
    } else {
      FillVisitList(mpq, intNode->left, visitlist, visitcount,
                    &leftIndexedRegion, max_distance, User_Distance);
    }
  }

  rightIndexedRegion = *indexedRegion;
  rightIndexedRegion.Low(intNode->splitDim, intNode->splitPos2);

  // query overlaps with right partition
  if (rightIndexedRegion.Overlap(mpq, max_distance, User_Distance)) {
    if (intNode->right >= OFNODECARD) {
      if (leaf[intNode->right - OFNODECARD].elsvalid) {
        liveRegion.DecodeLiveSpace(leaf[intNode->right - OFNODECARD].elsarray,
                                   &rightIndexedRegion);
        if (liveRegion.Overlap(mpq, max_distance, User_Distance)) {
          visitlist[*visitcount].index = (intNode->right) - OFNODECARD;
          visitlist[*visitcount].indexedRegion = rightIndexedRegion;
          (*visitcount)++;
        }
      } else {
        visitlist[*visitcount].index = (intNode->right) - OFNODECARD;
        visitlist[*visitcount].indexedRegion = rightIndexedRegion;
        (*visitcount)++;
      }
    } else {
      FillVisitList(mpq, intNode->right, visitlist, visitcount,
                    &rightIndexedRegion, max_distance, User_Distance);
    }
  }
}

// ********************** END: CODE FOR MULTIPOINT QUERIES *******************
// ********************** END: CODE FOR MULTIPOINT QUERIES *******************
// ********************** END: CODE FOR MULTIPOINT QUERIES *******************
