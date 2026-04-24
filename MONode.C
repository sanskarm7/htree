// FILENAME: MONode.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the MONode of the hybrid tree.
// implements functions to add a child to the monode,
// split the MOnode in case of an overflow etc.
// ------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "MONode.h"



int leftboundcmp(struct lineseg *a, struct lineseg *b)
{
  if (a->leftbound < b->leftbound) return -1;
  else if (a->leftbound == b->leftbound ) return 0;
  else return 1;
}


int rightboundcmp(struct lineseg *a, struct lineseg *b)
{
  if (a->rightbound > b->rightbound ) return -1;
  else if (a->rightbound == b->rightbound ) return 0;
  else return 1;
}


int overlapcmp(struct MOnode_split_info *a, struct MOnode_split_info *b)
{
  if (a->amountOfOverlap < b->amountOfOverlap) return -1;
  else if (a->amountOfOverlap == b->amountOfOverlap) return 0;
  else return 1;
}

int IsPresent(unsigned int aLeaf, unsigned int *leafList, int leafCount)
{
  int i;

  for(i=0; i<leafCount; i++)
    if (leafList[i]==aLeaf) return 1;
  return 0;
}


MONode::MONode(): Node(Node::MONODE) 
{
  int i;
  numInternalNodes = 0;
  numLeafNodes = 0;
  for (i = 0; i < MONODECARD-1; i++)
    InternalInit(i);
  for (i = 0; i < MONODECARD; i++)
    LeafInit(i);
}


MONode::~MONode(){
}


void MONode::Init()
{
  int i;
  numInternalNodes = 0;
  numLeafNodes = 0;
  for (i = 0; i < MONODECARD-1; i++)
    InternalInit(i);
  for (i = 0; i < MONODECARD; i++)
    LeafInit(i);
}

void MONode::LeafInit(int i)
{
  leaf[i].parent=0;
  // leaf[i].radius=0.0;
  leaf[i].child=NULL;
}

void MONode::InternalInit(int i)
{
  internal[i].splitDim=0;
  internal[i].splitPos1=0.0;
  internal[i].splitPos2=0.0;
  internal[i].left=0;
  internal[i].right=0;

}

void MONode::FillPickList(Point *r, int currRoot, struct kdTreeLeafInfo *picklist, int *pickcount, Rect *indexedRegion)
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node


  intNode = &(internal[currRoot]);
  if (r->position[intNode->splitDim] <= intNode->splitPos1) // point to be inserted  
							   // overlaps with left partition
    {
      leftIndexedRegion=*indexedRegion;
      leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;
      if (intNode->left >= MONODECARD)
	{
	  picklist[*pickcount].leafNodeIndex=(intNode->left)-MONODECARD;
	  picklist[*pickcount].leafRect=leftIndexedRegion;
	  (*pickcount)++;
	}
      else
	{
	  FillPickList(r, intNode->left, picklist, pickcount, &leftIndexedRegion);
	}
    }

  if (r->position[intNode->splitDim] > intNode->splitPos2) // point to be inserted
                                                        // overlaps with right partition
    {
      rightIndexedRegion=*indexedRegion;
      rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;
      if (intNode->right >= MONODECARD)
	{
	  picklist[*pickcount].leafNodeIndex=(intNode->right)-MONODECARD;
	  picklist[*pickcount].leafRect=rightIndexedRegion;
	  (*pickcount)++;
	}
      else
	{
	  FillPickList(r, intNode->right, picklist, pickcount, &rightIndexedRegion);
	}
    }
}


// Pick the branch in which the point should be inserted.  
// Pick the one which needs minimum enlargement. 
// Returns the index of the leafnode of the kdtree into 
// which the point should be inserted
//
int MONode::PickChild(Point *R, Rect *parentNodeRect, Rect *childNodeRect)
{
  Point *r = R;
  struct kdTreeLeafInfo picklist[MONODECARD];
  int pickcount=0;
  float minvol = 99999999.0;
  float volume;
  int i;
  int bestPick=-1;         // kdtreeLeafNode index for the best pick
  FillPickList(r, 0, picklist, &pickcount, parentNodeRect);
  
  assert(pickcount >0);  // at least there is one leaf overlapping with the point

  if (pickcount == 1)  // point overlaps with only one node, this is the leaf
    {
      (*childNodeRect)=picklist[0].leafRect;
      assert(picklist[0].leafNodeIndex < numLeafNodes);
      return (picklist[0].leafNodeIndex);
    }
  
  else 
    // pickcount > 1, point overlaps with many nodes, all need zero enlargement, 
    // resolve ties using size of rectangle, choose the one with the smallest size 
    {
      /*
      for(i=0; i<pickcount; i++)
	{
	  volume=(picklist[i].leafRect).Volume();
	  if (volume < minvol)
	    {
	      bestPick = i;
	      minvol = volume;
	    }
	}
      */
      // choose it arbitrarily, don't try to compute volume
      bestPick=(int)(drand48()*pickcount);
    }      
      assert(bestPick >=0);
      (*childNodeRect)=picklist[bestPick].leafRect;
      assert(picklist[bestPick].leafNodeIndex < numLeafNodes);
      return (picklist[bestPick].leafNodeIndex);
}
  

// Add a branch to a MO node.  Split the node if necessary.
// Returns 0 if node not split.  Old node updated.
// Returns 1 if node split, sets *new_node to address of new node.
// Old node updated, becomes one of two.
//
int MONode::AddChild(Node *newChildNode, MONode **New_node, int kdTreeLeafIndex, struct split* childNodeSplit, struct split **kdTreeSplit, Rect *indexedRegion)
{	
  MONode **new_node = New_node;
  int i, parent;


  parent=leaf[kdTreeLeafIndex].parent;
  if (internal[parent].left == (kdTreeLeafIndex+MONODECARD))
    internal[parent].left = numInternalNodes;
  else if (internal[parent].right == (kdTreeLeafIndex+MONODECARD))
    internal[parent].right = numInternalNodes;
  else
    assert(FALSE);   // shouldnt happen

  leaf[kdTreeLeafIndex].parent = numInternalNodes ;

  internal[numInternalNodes].splitDim= childNodeSplit->splitDim;
  internal[numInternalNodes].splitPos1= childNodeSplit->splitPos;
  internal[numInternalNodes].splitPos2= childNodeSplit->splitPos2;
  internal[numInternalNodes].left=MONODECARD+kdTreeLeafIndex;
  internal[numInternalNodes].right=MONODECARD+numLeafNodes;
  internal[numInternalNodes].parent=parent;
  leaf[numLeafNodes].child = newChildNode;
  leaf[numLeafNodes].parent = numInternalNodes ;

  numInternalNodes++;                        
  numLeafNodes++;                        
  
  if (numLeafNodes >= MONODECARD)	// the Monode is full, should be split
    {
      SplitNode(new_node, kdTreeSplit, indexedRegion);
      return 1;
    } 

  else return 0; 
}
  

// get rect's of all children
void MONode::FillChildList(int currRoot, Rect *childRectlist,  Rect *indexedRegion)
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node

  if (numLeafNodes <= 0) return;
  intNode = &(internal[currRoot]);

  leftIndexedRegion=*indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;

  if (intNode->left >= MONODECARD)
    {
      childRectlist[(intNode->left)-MONODECARD]=leftIndexedRegion;
    }
  else
    {
      FillChildList(intNode->left, childRectlist, &leftIndexedRegion);
    }
 
  rightIndexedRegion=*indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;

  if (intNode->right >= MONODECARD)
    {
      childRectlist[(intNode->right)-MONODECARD]=rightIndexedRegion;
    }
  else
    {
      FillChildList(intNode->right, childRectlist, &rightIndexedRegion);
    }
}



// bounding box search

void MONode::FillVisitList(Rect *r, int currRoot, int *visitlist, int *visitcount)
{
  struct OverlappingkdTreeInternalNode *intNode;

  if (numLeafNodes <= 0)
      {
       *visitcount=0;
       return;
      }
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node
  intNode = &(internal[currRoot]);
  if (r->boundary[intNode->splitDim] < intNode->splitPos1) //query rect. overlaps 
							    // with left partition
    {
      if (intNode->left >= MONODECARD)
	{
	  visitlist[*visitcount]=(intNode->left)-MONODECARD;
	  (*visitcount)++;
	}
      else
	{
	  FillVisitList(r, intNode->left, visitlist, visitcount);
	}
    }

  if (r->boundary[(intNode->splitDim)+NUMDIMS] > intNode->splitPos2) // query rect overlaps
                                                        // with right partition
    {
      if (intNode->right >= MONODECARD)
	{
	  visitlist[*visitcount]=(intNode->right)-MONODECARD;
	  (*visitcount)++;
	}
      else
	{
	  FillVisitList(r, intNode->right, visitlist, visitcount);
	}
    }
}


// splitting functions for MONode



// the following set of functions are to split a MO node
// the MO node (the lowest level MO node is actually OF because the
// level below it is OF and a OF node is split in an OF manner)
// is split in a MO manner, there is no question of
// cascading splits. cascading splits is avoided because the cascade 
// is no longer retricted to one level since there many levels below the
// level of MO node. we choose the split with minimum amount
// of overlap which satisfies the min. utilization


void MONode::GetkdtreeLeafBRs(int subtreeroot, Rect *indexedRegion, Rect *kdtreeLeafBRList)
{
  Rect leftRegion, rightRegion;
  unsigned short thissplitDim;
  float leftsplitPos, rightsplitPos;
  if (subtreeroot >= MONODECARD) 
    {
      kdtreeLeafBRList[subtreeroot-MONODECARD]=(*indexedRegion);
    }
  else /* internal node */
    {
      thissplitDim=internal[subtreeroot].splitDim;
      leftsplitPos=internal[subtreeroot].splitPos1;
      rightsplitPos=internal[subtreeroot].splitPos2;

      leftRegion=*indexedRegion;
      leftRegion.boundary[NUMDIMS+thissplitDim]=leftsplitPos;
      GetkdtreeLeafBRs(internal[subtreeroot].left, &leftRegion, kdtreeLeafBRList);
      
      rightRegion=*indexedRegion;
      rightRegion.boundary[thissplitDim]=rightsplitPos;
      GetkdtreeLeafBRs(internal[subtreeroot].right, &rightRegion, kdtreeLeafBRList);
    }
}



void MONode::GetNumLeavesInMOSubTree(int index, int *numLeaves, unsigned int *listOfLeaves)
{
  if (index < MONODECARD) /* internal node */
    {
      GetNumLeavesInMOSubTree(internal[index].left, numLeaves, listOfLeaves);
      GetNumLeavesInMOSubTree(internal[index].right, numLeaves, listOfLeaves);
    }
  else /* leaf node */
    {
      listOfLeaves[(*numLeaves)]=(index-MONODECARD);
      (*numLeaves)++;
    }
}


void MONode::GetMOSplitInfo(unsigned int subtreeroot, unsigned int stopnode, unsigned short splitdim, float splitpos1, float splitpos2, struct MOnode_split_info *newSplitInfo)
{
  if (subtreeroot >= MONODECARD) 
    {
      newSplitInfo->listOfSplittingChildren[newSplitInfo->numCascadingSplits]=subtreeroot-MONODECARD;
      newSplitInfo->numCascadingSplits++;
    }
  else /* internal node */
    {
      if (internal[subtreeroot].splitDim != splitdim)
	{
	  GetMOSplitInfo(internal[subtreeroot].left, stopnode, splitdim, splitpos1, splitpos2, newSplitInfo);
	  
	  GetMOSplitInfo(internal[subtreeroot].right, stopnode, splitdim, splitpos1, splitpos2, newSplitInfo);
	}
      else /* n->rest.monode.internal[subtreeroot].splitDim == splitdim */
	{
	  if (internal[subtreeroot].splitPos1 <= splitpos2)
	    GetNumLeavesInMOSubTree(internal[subtreeroot].left, &(newSplitInfo->numLeftChildren), newSplitInfo->listOfLeftChildren);
	  else
	    GetMOSplitInfo(internal[subtreeroot].left, stopnode, splitdim, splitpos1, splitpos2, newSplitInfo);
	  
	  if (internal[subtreeroot].splitPos2 >= splitpos1)
	    GetNumLeavesInMOSubTree(internal[subtreeroot].right, &(newSplitInfo->numRightChildren), newSplitInfo->listOfRightChildren);
	  else
	    GetMOSplitInfo(internal[subtreeroot].right, stopnode, splitdim, splitpos1, splitpos2, newSplitInfo);
	    }
    }
}



/* return split information i.e. number of cascading splits,
   utilization etc. if internal[index] is used as the split
   hyperplane 
*/

struct MOnode_split_info MONode::GetMONodeSplitInfo(int index)
{
  struct MOnode_split_info newSplitInfo;
  unsigned short splitdim= internal[index].splitDim;
  float splitpos1= internal[index].splitPos1;
  float splitpos2= internal[index].splitPos2;
  /* initialize */
  // newSplitInfo.splitNode=index;
  newSplitInfo.splitdim=splitdim;
  newSplitInfo.numCascadingSplits=0;
  newSplitInfo.numLeftChildren=0;
  newSplitInfo.numRightChildren=0;

  if (index == 0) // root of the kd-tree
    {
      GetNumLeavesInMOSubTree(internal[index].left, &(newSplitInfo.numLeftChildren), newSplitInfo.listOfLeftChildren);
      GetNumLeavesInMOSubTree(internal[index].right,&(newSplitInfo.numRightChildren), newSplitInfo.listOfRightChildren);
    }
  else
    {
      GetMOSplitInfo(0, index, splitdim, splitpos1, splitpos2, &newSplitInfo);
    }
  assert((newSplitInfo.numRightChildren+newSplitInfo.numLeftChildren+newSplitInfo.numCascadingSplits)== MONODECARD);
  return newSplitInfo;
}



int MONode::IsAncestor(unsigned int internalNodeIndex, unsigned int leafNodeIndex)
{
  unsigned int ancestor;
  ancestor=leaf[leafNodeIndex].parent;

  for(;;)
    {
      if (ancestor >= MONODECARD) break; // reached the root of the kd-tree
      if (ancestor == internalNodeIndex) return 1; // the internal node is an ancestor of the leaf 
      ancestor=internal[ancestor].parent;
    }
  return 0;
}

struct numleaves MONode::GetNumLeavesInSubtree(struct MOnode_split_info *splitInfo, unsigned int subtreeroot, int option)
{
  struct numleaves leafInfo;
  int leaf, i;
  int newsubtreeroot;

  leafInfo.numLeavesInSubtree=0;
  leafInfo.numLeavesUnderLeftChild=0;  
  

  if (option == LEFT)
    {
      for(i=0; i< splitInfo->numLeftChildren; i++)
	{
	  leaf = splitInfo->listOfLeftChildren[i];
	  if (IsAncestor(subtreeroot, leaf))
	    {
	      (leafInfo.numLeavesInSubtree)++;
	      leafInfo.leafNode=leaf;
	    }
	}

      if (leafInfo.numLeavesInSubtree>1)
	{
	  newsubtreeroot=internal[subtreeroot].left;
	  if (newsubtreeroot >= MONODECARD) 
	    {
	      for(i=0; i< splitInfo->numLeftChildren; i++)
		{
		  if (splitInfo->listOfLeftChildren[i]==(newsubtreeroot-MONODECARD))
		    {
		      leafInfo.numLeavesUnderLeftChild=1;
		      break;
		    }
		}
	    }
	  else
	    {
	      for(i=0; i< splitInfo->numLeftChildren; i++)
		{
		  leaf = splitInfo->listOfLeftChildren[i];
		  if (IsAncestor(newsubtreeroot, leaf))
		      (leafInfo.numLeavesUnderLeftChild)++;
		}
	    }
	}
    }

  if (option == RIGHT)
    {
      for(i=0; i< splitInfo->numRightChildren; i++)
	{
	  leaf = splitInfo->listOfRightChildren[i];
	  if (IsAncestor(subtreeroot, leaf))
	    {
	      (leafInfo.numLeavesInSubtree)++;
	      leafInfo.leafNode=leaf;
	    }
	}

      if (leafInfo.numLeavesInSubtree>1)
	{
	  newsubtreeroot=internal[subtreeroot].left;
	  if (newsubtreeroot >= MONODECARD) 
	    {
	      for(i=0; i< splitInfo->numRightChildren; i++)
		{
		  if (splitInfo->listOfRightChildren[i]==(newsubtreeroot-MONODECARD))
		    {
		      leafInfo.numLeavesUnderLeftChild=1;
		      break;
		    }
		}
	    }
	  else
	    {
	      for(i=0; i< splitInfo->numRightChildren; i++)
		{
		  leaf = splitInfo->listOfRightChildren[i];
		  if (IsAncestor(newsubtreeroot, leaf))
		      (leafInfo.numLeavesUnderLeftChild)++;
		}
	    }
	}
    }
  return leafInfo;
}
  


int MONode::GetNextMOkdTreeNode(int option, int currnodeptr, struct MOnode_split_info *splitInfo)
{
  int ptr = currnodeptr;
  struct numleaves leafInfo;

  assert(ptr < (MONODECARD-1));   /* pointing to an internal node */

  for(;;)
    {
      if (ptr >= MONODECARD) return ptr;    /* pointing to a leaf node */

      if (option == LEFT)
	{
	  if (internal[ptr].splitDim == splitInfo->splitdim)
	    {
	      if (internal[ptr].splitPos2 >= splitInfo->splitpos1)
		{
		  ptr=internal[ptr].left;
		  continue;
		}
	      if (internal[ptr].splitPos2 < splitInfo->splitpos2)
		return ptr; 
	    }

	  leafInfo=GetNumLeavesInSubtree(splitInfo, ptr, LEFT);
	  if (leafInfo.numLeavesInSubtree==1) 
	    {
	      ptr=leafInfo.leafNode+MONODECARD;
	      return ptr;
	    }
	  else
	    {
	      if (leafInfo.numLeavesUnderLeftChild == 0)
		ptr=internal[ptr].right;

	      else if (leafInfo.numLeavesUnderLeftChild == leafInfo.numLeavesInSubtree)
		ptr=internal[ptr].left;

	      else return ptr;
	    }
	}

      if (option == RIGHT)
	{
	  
	  if (internal[ptr].splitDim == splitInfo->splitdim)
	    {
	      if (internal[ptr].splitPos1 <= splitInfo->splitpos2)
		{
		  ptr=internal[ptr].right;
		  continue;
		}
	      if (internal[ptr].splitPos1 > splitInfo->splitpos1)
		return ptr;
	    }

	  leafInfo=GetNumLeavesInSubtree(splitInfo, ptr, RIGHT);
	  
	  if (leafInfo.numLeavesInSubtree==1) 
	    {
	      ptr=leafInfo.leafNode+MONODECARD;
	      return ptr;
	    }

	  else
	    {
	      if (leafInfo.numLeavesUnderLeftChild == 0)
		ptr=internal[ptr].right;

	      else if (leafInfo.numLeavesUnderLeftChild == leafInfo.numLeavesInSubtree)
		ptr=internal[ptr].left;

	      else return ptr;
	    }
	}
    }
}



void MONode::ConstructMOSubtree(MONode *subtreeNode, struct MOnode_split_info *splitInfo, int option)
{
  int target, leftchild, rightchild;
  int rootptr;
  int ptr=0;

  rootptr=0;           /* initially points to the root of the kdtree */

  rootptr=GetNextMOkdTreeNode(option, rootptr, splitInfo);  /* get the root of the kdtree being constructed */
     
  /* pointing to an internal node */
  assert(rootptr < MONODECARD);
    
  subtreeNode->internal[0]=internal[rootptr];
  subtreeNode->internal[0].parent=MONODECARD;
  subtreeNode->numInternalNodes++;
  
  while(ptr < subtreeNode->numInternalNodes)
    {
      leftchild=(subtreeNode->internal[ptr]).left;
      if (leftchild < MONODECARD) leftchild=GetNextMOkdTreeNode(option, leftchild, splitInfo);
      if ( leftchild >= MONODECARD) // left child of current node is a leaf node
	{
          assert( (leftchild-MONODECARD) < numLeafNodes);
	  target=subtreeNode->numLeafNodes++;
	  subtreeNode->leaf[target] = leaf[leftchild-MONODECARD];
	  subtreeNode->internal[ptr].left=target+MONODECARD;
	  subtreeNode->leaf[target].parent = ptr;
	}
      else // left child of current node is an internal node
	{
          assert( leftchild < numInternalNodes);
	  target=subtreeNode->numInternalNodes++;
	  subtreeNode->internal[target] = internal[leftchild];
	  subtreeNode->internal[ptr].left=target;
	  subtreeNode->internal[target].parent = ptr;
	}
      
      rightchild=(subtreeNode->internal[ptr]).right;
      if (rightchild < MONODECARD) rightchild=GetNextMOkdTreeNode(option, rightchild, splitInfo);
      if ( rightchild >= MONODECARD) // right child of current node is a leaf node
	{
          assert( (rightchild-MONODECARD) < numLeafNodes);
	  target=subtreeNode->numLeafNodes++;
	  subtreeNode->leaf[target] = leaf[rightchild-MONODECARD];
	  subtreeNode->internal[ptr].right=target+MONODECARD;
	  subtreeNode->leaf[target].parent = ptr;
	}
      else // right child of current node is an internal node
	{
	  assert( rightchild < numInternalNodes);
	  target=subtreeNode->numInternalNodes++;
	  subtreeNode->internal[target] = internal[rightchild];
	  subtreeNode->internal[ptr].right=target;
	  subtreeNode->internal[target].parent = ptr;
	}
      ptr ++;
    }
}


void MONode::SplitMONodekdTree(MONode **nn, struct MOnode_split_info *splitInfo)
{
  MONode *replacementNode;

  (*nn)=new MONode(); 
  replacementNode=new MONode(); 
  (*nn)->level=level;
  replacementNode->level=level;

  assert(splitInfo->numRightChildren+splitInfo->numLeftChildren== MONODECARD);
    
  ConstructMOSubtree(replacementNode, splitInfo, LEFT);   /* construct left subtree */
  ConstructMOSubtree(*nn, splitInfo, RIGHT);               /* construct right subtree */
  
  assert ((replacementNode->numLeafNodes+(*nn)->numLeafNodes)== MONODECARD); 
  
  // printf("splitdimension = %d before split numleaves = %d left node numleaves = %d rightnode numleaves = %d \n amount of overlap=%f", splitInfo->splitdim, numLeafNodes, replacementNode->numLeafNodes, (*nn)->numLeafNodes, (splitInfo->splitpos1-splitInfo->splitpos2));
  // getchar();
  
  CopyFrom(replacementNode);
  delete replacementNode;
}  




void MONode::GuaranteedAssessSplits(struct MOnode_split_info *split_choice_array, int numPossibleSplitDims, Rect *kdtreeLeafBRList, Rect *indexedRegion)
{
  unsigned short splitDimension;
  float splitPosition1;
  float splitPosition2;
  float newsplitPosition1, newsplitPosition2;
  Rect OverlappingRegion, leafBR;
  struct lineseg leftBounds[MONODECARD];
  struct lineseg rightBounds[MONODECARD];
  int i,j, k, limit, leftptr, rightptr;
  int splittingLeaf;

  for(i=0; i< numPossibleSplitDims; i++)
    {
      splitDimension=split_choice_array[i].splitdim;
      // splitDimension=i;
      // split_choice_array[i].splitdim=splitDimension;

      // project the leaf objects in this dimension, then sort those line
      // segments based on their left boundaries, and also based on their 
      // right boundaries

      for(k=0; k<MONODECARD; k++)
	{
	  leftBounds[k].leafNodeIndex=k;
	  rightBounds[k].leafNodeIndex=k;
	  leftBounds[k].leftbound=kdtreeLeafBRList[k].boundary[splitDimension];
	  leftBounds[k].rightbound=kdtreeLeafBRList[k].boundary[splitDimension+NUMDIMS];
	  rightBounds[k].leftbound=kdtreeLeafBRList[k].boundary[splitDimension];
	  rightBounds[k].rightbound=kdtreeLeafBRList[k].boundary[splitDimension+NUMDIMS];
	}

      qsort(leftBounds, MONODECARD, sizeof(struct lineseg), (int (*) (const void *, const void *)) leftboundcmp);
      
      qsort(rightBounds, MONODECARD, sizeof(struct lineseg), (int (*) (const void *, const void *)) rightboundcmp);

      limit=(int)(MONODECARD*MINMONODEUTIL)+1;
      
      split_choice_array[i].numLeftChildren=0;
      split_choice_array[i].numRightChildren=0;
      split_choice_array[i].numCascadingSplits=0;
      splitPosition1=-99999.99;
      splitPosition2=99999.99;

      leftptr=0;
      rightptr=0;

      for(k=0;k<MONODECARD;k++)
	{
	  // select a left guy
	  
	  for(; leftptr< MONODECARD;leftptr++)
	    if (!IsPresent(leftBounds[leftptr].leafNodeIndex, split_choice_array[i].listOfRightChildren, split_choice_array[i].numRightChildren)) break;

	  split_choice_array[i].listOfLeftChildren[split_choice_array[i].numLeftChildren++]=leftBounds[leftptr].leafNodeIndex;
	  splitPosition1=MAX(splitPosition1, leftBounds[leftptr].rightbound);
	  leftptr++;

	  // select a right guy
	  for(; rightptr< MONODECARD; rightptr++)
	    if (!IsPresent(rightBounds[rightptr].leafNodeIndex, split_choice_array[i].listOfLeftChildren, split_choice_array[i].numLeftChildren)) break;
	    
	  split_choice_array[i].listOfRightChildren[split_choice_array[i].numRightChildren++]=rightBounds[rightptr].leafNodeIndex;
	  splitPosition2=MIN(splitPosition2, rightBounds[rightptr].leftbound);
	  rightptr++;
	    
	  if (MIN(split_choice_array[i].numLeftChildren, split_choice_array[i].numRightChildren) >= limit) break;
	}


      // put the rest in the cascading splits section, distribute without 
      // being concerned about utilization, already taken care of
      for(k=0; k<MONODECARD; k++)
	{
	  if (!IsPresent(k, split_choice_array[i].listOfLeftChildren, split_choice_array[i].numLeftChildren) && !IsPresent(k, split_choice_array[i].listOfRightChildren, split_choice_array[i].numRightChildren))
	    {
	      split_choice_array[i].listOfSplittingChildren[split_choice_array[i].numCascadingSplits++]=k;
	    }
	}
      assert((split_choice_array[i].numRightChildren+split_choice_array[i].numLeftChildren+split_choice_array[i].numCascadingSplits)== MONODECARD);

      newsplitPosition1=splitPosition1;
      newsplitPosition2=splitPosition2;
      

      for(j=0; j<split_choice_array[i].numCascadingSplits; j++)
	{
	  splittingLeaf=split_choice_array[i].listOfSplittingChildren[j];
	  leafBR=kdtreeLeafBRList[splittingLeaf];
	  if ((leafBR.boundary[splitDimension+NUMDIMS]-splitPosition1) < (splitPosition2-leafBR.boundary[splitDimension])) // the leaf should go to the left partition
	    {
	      newsplitPosition1=MAX(newsplitPosition1, leafBR.boundary[splitDimension+NUMDIMS]);
	      split_choice_array[i].listOfLeftChildren[split_choice_array[i].numLeftChildren]=splittingLeaf;
	      split_choice_array[i].numLeftChildren++;
	    }
	  else if ((leafBR.boundary[splitDimension+NUMDIMS]-splitPosition1) > (splitPosition2-leafBR.boundary[splitDimension]))
	    // the leaf should go to the right partition
	    {
	      newsplitPosition2=MIN(newsplitPosition2, leafBR.boundary[splitDimension]);
	      split_choice_array[i].listOfRightChildren[split_choice_array[i].numRightChildren]=splittingLeaf;
	      split_choice_array[i].numRightChildren++;
	    }
	  // resolve ties by putting into the partition that has less people
	  else // the elongation needed is equal
	    {
	      if (split_choice_array[i].numLeftChildren >= split_choice_array[i].numRightChildren)
		{
		  newsplitPosition2=MIN(newsplitPosition2, leafBR.boundary[splitDimension]);
		  split_choice_array[i].listOfRightChildren[split_choice_array[i].numRightChildren]=splittingLeaf;
		  split_choice_array[i].numRightChildren++;
		}
	      else
		{
		  newsplitPosition1=MAX(newsplitPosition1, leafBR.boundary[splitDimension+NUMDIMS]);
		  split_choice_array[i].listOfLeftChildren[split_choice_array[i].numLeftChildren]=splittingLeaf;
		  split_choice_array[i].numLeftChildren++;
		  
		}
	    }
	}

      assert((split_choice_array[i].numRightChildren+split_choice_array[i].numLeftChildren)== MONODECARD);
      // OverlappingRegion=(*indexedRegion);
      // OverlappingRegion.boundary[splitDimension]=newsplitPosition2;
      // OverlappingRegion.boundary[splitDimension+NUMDIMS]=newsplitPosition1;
      // split_choice_array[i].amountOfOverlap=RTreeRectVolume(&OverlappingRegion);
      split_choice_array[i].amountOfOverlap=newsplitPosition1-newsplitPosition2;
      split_choice_array[i].splitpos1=newsplitPosition1;
      split_choice_array[i].splitpos2=newsplitPosition2;
      assert ((split_choice_array[i]).numRightChildren + (split_choice_array[i]).numLeftChildren == MONODECARD);
    }
}




void MONode::SplitNode(MONode **nn, struct split **newSplit, Rect *indexedRegion)
{
  int i,h,z;
  unsigned int splitnode;
  Rect OverlappingRegion;
  unsigned int splitDimension;
  struct MOnode_split_info split_choice_array[NUMDIMS];
  float worst_case_util;
  unsigned int listOfPossibleSplitDims[NUMDIMS];
  int numPossibleSplitDims=0;
  Rect kdtreeLeafBRList[MONODECARD]; 
  
  assert(numInternalNodes == (MONODECARD-1)); // node is full, hence splitting
  GetkdtreeLeafBRs(0, indexedRegion, kdtreeLeafBRList);
 

  // what happens if we split by the root
  split_choice_array[0]= GetMONodeSplitInfo(0);

  // check whether possible to split by root (i.e. preserving utilization constraints)

  assert(split_choice_array[0].numCascadingSplits == 0);
  assert((split_choice_array[0]).numRightChildren + (split_choice_array[0]).numLeftChildren == MONODECARD);


  worst_case_util=MIN(((float)(split_choice_array[0].numLeftChildren)/(MONODECARD)),((float)(split_choice_array[0].numRightChildren)/(MONODECARD)));
  if (split_choice_array[0].numLeftChildren>1 && split_choice_array[0].numRightChildren> 1 && worst_case_util > MINMONODEUTIL)
    {
      i=0;
      split_choice_array[0].splitpos1=internal[0].splitPos1;
      split_choice_array[0].splitpos2=internal[0].splitPos2;
      splitDimension=split_choice_array[0].splitdim;
      // OverlappingRegion=(*indexedRegion);
      // OverlappingRegion.boundary[splitDimension]=split_choice_array[0].splitpos2;
      // OverlappingRegion.boundary[splitDimension+NUMDIMS]=split_choice_array[0].splitpos1;
      // split_choice_array[0].amountOfOverlap=RTreeRectVolume(&OverlappingRegion);
      split_choice_array[0].amountOfOverlap=split_choice_array[0].splitpos1-split_choice_array[0].splitpos2;
    }
  else
    {
      // consider only the dimensions with which this node has been split
      numPossibleSplitDims=0;
      for(h=0; h<MONODECARD-1; h++)
	{
	  if (!IsPresent(internal[h].splitDim, listOfPossibleSplitDims, numPossibleSplitDims))
	    {
	      listOfPossibleSplitDims[numPossibleSplitDims]=internal[h].splitDim;
	      split_choice_array[numPossibleSplitDims].splitdim=internal[h].splitDim;
	      numPossibleSplitDims++;
	    }
	}

      GuaranteedAssessSplits(split_choice_array, numPossibleSplitDims, kdtreeLeafBRList, indexedRegion);
      
      qsort(split_choice_array, numPossibleSplitDims, sizeof(struct MOnode_split_info), (int (*) (const void *, const void *)) overlapcmp);
      
      for(i=0; i<numPossibleSplitDims; i++)
	{
	 if (split_choice_array[i].numLeftChildren > 1 && split_choice_array[i].numRightChildren > 1)
	 {
	  worst_case_util=MIN(((float)(split_choice_array[i].numLeftChildren)/(MONODECARD)),((float)(split_choice_array[i].numRightChildren)/(MONODECARD)));
	  
	  if (worst_case_util > MINMONODEUTIL) 
	    {
	      break;
	    }
	}
       }
    } 

  (*newSplit) = (struct split *) malloc (sizeof(struct split));
  (*newSplit)->splitDim=split_choice_array[i].splitdim;
  (*newSplit)->splitPos=split_choice_array[i].splitpos1;
  (*newSplit)->splitPos2=split_choice_array[i].splitpos2;

  SplitMONodekdTree(nn, &(split_choice_array[i]));
  assert((numLeafNodes+(*nn)->numLeafNodes)== MONODECARD);
#ifdef HTREE_DEBUG
  printf("MONode split (***** Minimal Overlap *****): split dimension = %d split position left = %f split position right = %f\n", split_choice_array[i].splitdim, split_choice_array[i].splitpos1, split_choice_array[i].splitpos2);
#endif
}




void MONode::CreatekdTree(Node *leftChildNode, Node *rightChildNode, struct split *newSplit)
{
  numInternalNodes=1;
  numLeafNodes=2;
  internal[0].splitDim=newSplit->splitDim;
  internal[0].splitPos1=newSplit->splitPos;
  internal[0].splitPos2=newSplit->splitPos2;
  internal[0].left=MONODECARD;
  internal[0].right=MONODECARD+1;
  internal[0].parent=MONODECARD; // means junk, no parent
  leaf[0].parent=0;
  leaf[0].child=leftChildNode;
  leaf[1].parent=0;
  leaf[1].child=rightChildNode;
}



unsigned int MONode::GetSibling(unsigned int index, int option)
{
  unsigned int parent;

  if (option == LEAF) 
    {
      parent=leaf[index].parent;
      index+=MONODECARD;
    }
  else 
    parent=internal[index].parent;
  
  if (internal[parent].left == index) 
    return internal[parent].right;
  else
    {
      assert (internal[parent].right == index); 
      return internal[parent].left;
    }
}


int MONode::WhichChildOfParent(unsigned int index, int option)
{
  unsigned int parent;

  if (option == LEAF) 
    {
      parent=leaf[index].parent;
      index+=MONODECARD;
    }
  else parent=internal[index].parent;
  
  if (parent >= MONODECARD || parent < 0) return -1;

  if (internal[parent].left == index) 
    return LEFT;
  else
    {
      assert (internal[parent].right == index); 
      return RIGHT;
    }
}



int MONode::RemoveLeaf(int i)
{
  unsigned int parent, grandparent, parent2, parent3, sibling, rightchild, leftchild;
  int jj, kk;
  DataNode *datanode;
  
  parent=leaf[i].parent;
  sibling=GetSibling(i, LEAF);
  if (parent != 0)
    {
      grandparent=internal[parent].parent;
      
     if (WhichChildOfParent(parent, INTERNAL) == LEFT)
       internal[grandparent].left=sibling;
     else
       internal[grandparent].right=sibling;
     
     if (sibling >= MONODECARD)
       leaf[sibling-MONODECARD].parent=grandparent;
     else
       internal[sibling].parent=grandparent;
    }
  else
    {
      if (sibling>=MONODECARD) return 0;  // cannot remove leaf becuase there are only 2 leaves in the node
      else
	{
          assert(sibling == 1); // since sibling is the only son of the root which is an internal node
          internal[sibling].parent=MONODECARD; //  else make the sibling the new root
	}
    }

  // the i th slot among leaf nodes is free, so do compaction
  
  for(jj=i+1; jj < numLeafNodes; jj++)
    {
      leaf[jj-1]=leaf[jj];
      
     parent2=leaf[jj].parent;
     
     if (WhichChildOfParent(jj, LEAF) == LEFT)
       internal[parent2].left=(jj+MONODECARD-1);
     else
       internal[parent2].right=(jj+MONODECARD-1);
    }
  
  
 // the parent slot among internal nodes is free, so do compaction
  
  for(kk=parent+1; kk< numInternalNodes; kk++)
    {
      internal[kk-1]=internal[kk];
      
      parent3=internal[kk].parent;
     
      if (WhichChildOfParent(kk, INTERNAL) == LEFT)
	internal[parent3].left=kk-1;
      else
	internal[parent3].right=kk-1;
      
     leftchild=internal[kk-1].left;
     rightchild=internal[kk-1].right;
     
     if (leftchild >= MONODECARD)
       leaf[leftchild-MONODECARD].parent=kk-1;
     else
       internal[leftchild].parent=kk-1;
     
     if (rightchild >= MONODECARD)
       leaf[rightchild-MONODECARD].parent=kk-1;
     else
       internal[rightchild].parent=kk-1;
    }
  numInternalNodes--;
  numLeafNodes--;
 return 1;   // removeleaf successful
}


  
void MONode::CopyFrom(MONode *n)
{
  int i;
  numInternalNodes = n->numInternalNodes;
  numLeafNodes = n->numLeafNodes;
  for (i = 0; i < MONODECARD-1; i++)
    internal[i]=n->internal[i];
  for (i = 0; i < MONODECARD; i++)
    leaf[i]=n->leaf[i];
}


// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES ******************* 
// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES ******************* 
// ********************** BEGIN: CODE FOR MULTIPOINT QUERIES ******************* 

void MONode::FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, int dist_func)
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  float distance;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node

  intNode = &(internal[currRoot]);

  leftIndexedRegion=*indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;

  if (intNode->left >= MONODECARD)
    {
      distance=MPQ->Distance(&leftIndexedRegion, dist_func);
      queue->push(PQEntry((leaf[intNode->left-MONODECARD]).child, NODE, leftIndexedRegion, distance, (leaf[intNode->left-MONODECARD]).count, NULL));
    }
  else
    FillChildList(intNode->left, MPQ, &leftIndexedRegion, queue, dist_func);
  rightIndexedRegion=*indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;
  
  if (intNode->right >= MONODECARD)
    {
      distance=MPQ->Distance(&rightIndexedRegion,dist_func);
      queue->push(PQEntry((leaf[intNode->right-MONODECARD]).child, NODE, rightIndexedRegion, distance, (leaf[intNode->right-MONODECARD]).count, NULL));
    }
  else
    FillChildList(intNode->right, MPQ, &rightIndexedRegion, queue, dist_func);
}


void MONode::FillChildList(int currRoot, MultiPointQuery *MPQ, Rect *indexedRegion, PQ *queue, float (*User_Distance) (Point *, Point *))
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  Point centroid;
  float distance;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node

  intNode = &(internal[currRoot]);

  leftIndexedRegion=*indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;

  if (intNode->left >= MONODECARD)
    {
      distance=MPQ->Distance(&leftIndexedRegion, User_Distance);
      queue->push(PQEntry((leaf[intNode->left-MONODECARD]).child, NODE, leftIndexedRegion, distance, (leaf[intNode->left-MONODECARD]).count, NULL));
    }
  else
    FillChildList(intNode->left, MPQ, &leftIndexedRegion, queue, User_Distance);

  rightIndexedRegion=*indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;

  if (intNode->right >= MONODECARD)
    {
      distance=MPQ->Distance(&rightIndexedRegion,User_Distance);
      queue->push(PQEntry((leaf[intNode->right-MONODECARD]).child, NODE, rightIndexedRegion, distance, (leaf[intNode->right-MONODECARD]).count, NULL));
    }
  else
    FillChildList(intNode->right, MPQ, &rightIndexedRegion, queue, User_Distance);
}



// distance based search
// fill in the visit list i.e. list of children of an MO node 
// i.e. leaves of the kdtree of the MO node that overlap with search rect

void MONode::FillVisitList(MultiPointQuery *mpq, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, int dist_func)
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node

  if (numLeafNodes <= 0)
      {
       *visitcount=0;
       return;
      }

  intNode = &(internal[currRoot]);

  leftIndexedRegion=*indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;

  
  // query overlaps with left partition
  if (leftIndexedRegion.Overlap(mpq, max_distance, dist_func)) 
    {
      if (intNode->left >= MONODECARD)
	{
	      visitlist[*visitcount].index=(intNode->left)-MONODECARD;
	      visitlist[*visitcount].indexedRegion=leftIndexedRegion;
	      (*visitcount)++;
	}
      else
	{
	  FillVisitList(mpq, intNode->left, visitlist, visitcount, &leftIndexedRegion, max_distance, dist_func);
	}
    }

  rightIndexedRegion=*indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;
  
  // query overlaps with right partition
  if (rightIndexedRegion.Overlap(mpq, max_distance, dist_func)) 
    {
      if (intNode->right >= MONODECARD)
	{
	      visitlist[*visitcount].index=(intNode->right)-MONODECARD;
	      visitlist[*visitcount].indexedRegion=rightIndexedRegion;
	      (*visitcount)++;
	}
      else
	{
	  FillVisitList(mpq, intNode->right, visitlist, visitcount, &rightIndexedRegion, max_distance, dist_func);
	}
    }
}




// distance based search, user defined distance function
// fill in the visit list i.e. list of children of an MO node 
// i.e. leaves of the kdtree of the MO node that overlap with search rect

void MONode::FillVisitList(MultiPointQuery *mpq, int currRoot, struct region *visitlist, int *visitcount, Rect *indexedRegion, float max_distance, float (*User_Distance) (Point *, Point *))
{
  int i;
  struct OverlappingkdTreeInternalNode *intNode;
  Rect leftIndexedRegion;
  Rect rightIndexedRegion;
  Rect liveRegion;
  assert(currRoot < (MONODECARD-1));   // must be an internal kdtree node

  if (numLeafNodes <= 0)
      {
       *visitcount=0;
       return;
      }

  intNode = &(internal[currRoot]);

  leftIndexedRegion=*indexedRegion;
  leftIndexedRegion.boundary[NUMDIMS+(intNode->splitDim)]=intNode->splitPos1;

  
  // query overlaps with left partition
  if (leftIndexedRegion.Overlap(mpq, max_distance, User_Distance)) 
    {
      if (intNode->left >= MONODECARD)
	{
	      visitlist[*visitcount].index=(intNode->left)-MONODECARD;
	      visitlist[*visitcount].indexedRegion=leftIndexedRegion;
	      (*visitcount)++;
	}
      else
	{
	  FillVisitList(mpq, intNode->left, visitlist, visitcount, &leftIndexedRegion, max_distance, User_Distance);
	}
    }

  rightIndexedRegion=*indexedRegion;
  rightIndexedRegion.boundary[intNode->splitDim]=intNode->splitPos2;
  
  // query overlaps with right partition
  if (rightIndexedRegion.Overlap(mpq, max_distance, User_Distance)) 
    {
      if (intNode->right >= MONODECARD)
	{
	      visitlist[*visitcount].index=(intNode->right)-MONODECARD;
	      visitlist[*visitcount].indexedRegion=rightIndexedRegion;
	      (*visitcount)++;
	}
      else
	{
	  FillVisitList(mpq, intNode->right, visitlist, visitcount, &rightIndexedRegion, max_distance, User_Distance);
	}
    }
}




// ********************** END: CODE FOR MULTIPOINT QUERIES ******************* 
// ********************** END: CODE FOR MULTIPOINT QUERIES ******************* 
// ********************** END: CODE FOR MULTIPOINT QUERIES ******************* 


