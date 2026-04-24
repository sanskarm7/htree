// FILENAME: DataNode.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the DataNode of the hybrid tree.
// implements functions to add a branch to the datanode,
// split the datanode in case of an overflow etc.
// ------------------------------------------------------------

#include <assert.h>
#include <math.h>
#include <string.h>   /* for memmove */
#include <stdint.h> /* for intptr_t -- NOTE: assumes 64-bit platform (x86-64 Linux) */
#include <stdio.h>
#include <stdlib.h>

#include "DataNode.h"

DataNode::DataNode() : Node(Node::DATANODE) {
  int i;
  count = 0;
  for (i = 0; i < NODECARD; i++)
    BranchInit(i);
}

DataNode::~DataNode() {}

void DataNode::BranchInit(int i) {
  branch[i].point.Init();
  branch[i].child = NULL;
}

void DataNode::Init() {
  int i;
  count = 0;
  for (i = 0; i < NODECARD; i++)
    BranchInit(i);
}

int DataNode::Count() { return count; }

void DataNode::Count(int newcount) { count = newcount; }

void DataNode::IncrCount() { count++; }

struct Branch DataNode::Branch(int i) {
  assert(i < count);
  return branch[i];
}

void DataNode::Branch(int i, struct Branch b) { branch[i] = b; }

void DataNode::DisconnectBranch(int i) {
  int j;
  assert(i >= 0 && i < NODECARD);
  assert(branch[i].child);

  // do compaction
  memmove((void *)(&branch[i]), (void *)(&branch[i + 1]),
          (count - i - 1) * sizeof(struct Branch));
  count--;
  BranchInit(count);
}

void DataNode::Print() {
  int i;
  printf("NODE TYPE: %d  LEVEL=%d  COUNT=%d  \n", type, level, count);
  for (i = 0; i < count; i++) {
    printf("branch %d\n", i);
    /* casting pointer to intptr_t for printing -- assumes 64-bit (pointer == 8
     * bytes on x86-64) */
    printf("%ld ", (intptr_t)(branch[i].child));
    (branch[i].point).Print();
  }
}

Rect DataNode::Cover() {
  register int i, first_time = 1;
  Rect r;

  for (i = 0; i < count; i++)
    if (branch[i].child) {
      if (first_time) {
        r = (branch[i].point).ToRect();
        first_time = 0;
      } else
        r = r.CombineRect(&(branch[i].point));
    }
  return r;
}

int DataNode::AddChild(struct Branch *b, DataNode **new_node,
                       struct split **newsplit) {
  assert(b);

  branch[count] = *b;
  count++;

  if (count >= NODECARD) /* split it */
  {
    assert(new_node);
    SplitNode(new_node, newsplit);
    return 1;
  } else
    return 0;
}

int floatcmp(float *a, float *b) {
  if (*a < *b)
    return -1;
  else if (*a > *b)
    return 1;
  else
    return 0;
}

int splitcmp(struct dimspread *a, struct dimspread *b) {
  if (a->spread > b->spread)
    return -1;
  else if (a->spread < b->spread)
    return 1;
  else
    return 0;
}

struct split DataNode::MaxExtentBisectionSplit() {
  Rect coverRect;
  register int i, j, k;
  struct dimspread spreads[NUMDIMS];
  float projection[NODECARD];
  float maxextent = -1.0;
  struct split bestSplit;
  int minfill, maxfill, leftcount, newleftcount;
  float splitposition;

  coverRect = Cover();

  for (i = 0; i < NUMDIMS; i++) {
    spreads[i].dimension = i;
    spreads[i].spread = (coverRect.High(i) - coverRect.Low(i));
  }

  // sort, the dim with the max spread first
  qsort(spreads, NUMDIMS, sizeof(struct dimspread),
        (int (*)(const void *, const void *))splitcmp);

  for (k = 0; k < NUMDIMS; k++) {
    bestSplit.splitDim = spreads[k].dimension;

    // tentative splitposition
    splitposition = (coverRect.High(bestSplit.splitDim) +
                     coverRect.Low(bestSplit.splitDim)) /
                    2;

    for (j = 0; j < NODECARD; j++)
      projection[j] = branch[j].point.At(bestSplit.splitDim);
    // sort it
    qsort(projection, NODECARD, sizeof(float),
          (int (*)(const void *, const void *))floatcmp);

    leftcount = 0;
    for (j = 0; j < NODECARD; j++) {
      if (projection[j] >= splitposition)
        break;
      leftcount++;
    }
    // keep the tentative splitposition if it satisfies node utilization
    // guarantees otherwise replace it by the one closest it that does
    minfill = (int)(MINDATANODEUTIL * NODECARD);
    if (minfill < 1)
      minfill = 1; // minfill should be at least one
    maxfill = NODECARD - minfill;
    if (leftcount < minfill)
      splitposition = (projection[minfill - 1] + projection[minfill]) / 2;
    else if (leftcount > maxfill)
      splitposition = (projection[maxfill - 1] + projection[maxfill]) / 2;

    newleftcount = 0;
    for (j = 0; j < NODECARD; j++) {
      if (projection[j] >= splitposition)
        break;
      newleftcount++;
    }

    if (newleftcount >= minfill) {
      bestSplit.splitPos = splitposition;
      return bestSplit;
    }
  }
}

void DataNode::TransferFrom(DataNode *src) {
  int i;
  for (i = 0; i < src->count; i++) {
    branch[count] = src->branch[i];
    count++;
    assert(count <= NODECARD);
  }
}

void DataNode::FillReinsertList(ReinsertList *l) {
  int i;
  for (i = 0; i < count; i++) {
    l->objectList[l->numObjects].id = (long)(branch[i].child);
    l->objectList[l->numObjects].point = branch[i].point;
    l->numObjects++;
  }
}

void DataNode::SplitNode(DataNode **nn, struct split **newSplit) {
  unsigned short splitdimension;
  float splitposition;
  DataNode *replacementNode;
  int target;
  int i;
  struct split bestSplit;

  if (count != NODECARD)
    printf("count %d\n", count);
  // assert(count == NODECARD);

  // allocate replacement node and newnode
  replacementNode = new DataNode();
  replacementNode->level = level;
  (*nn) = new DataNode();
  (*nn)->level = level;

  bestSplit = MaxExtentBisectionSplit();
  (*newSplit) = (struct split *)malloc(sizeof(struct split));
  (*newSplit)->splitDim = bestSplit.splitDim;
  (*newSplit)->splitPos = bestSplit.splitPos;
  (*newSplit)->splitPos2 = bestSplit.splitPos;

  splitdimension = bestSplit.splitDim;
  splitposition = bestSplit.splitPos;

  for (i = 0; i < NODECARD; i++) {
    if (branch[i].point.At(splitdimension) < splitposition) {
      target = replacementNode->Count();
      replacementNode->Branch(target, branch[i]);
      replacementNode->IncrCount();
    } else {
      target = (*nn)->Count();
      (*nn)->Branch(target, branch[i]);
      (*nn)->IncrCount();
    }
  }
  assert(replacementNode->Count() > 0);
  assert((*nn)->Count() > 0);
  if ((*nn)->Count() == 0 || replacementNode->Count() == 0)
    printf("wrong \n");
  CopyFrom(replacementNode);
  delete replacementNode;
#ifdef HTREE_DEBUG
  // printf("DataNode split (***** Overlap Free *****): split dimension = %d
  // split position = %f \n", bestSplit.splitDim, bestSplit.splitPos);
#endif HTREE_DEBUG
}

int DataNode::ComputeNumReinsertions(unsigned int splitdimension,
                                     float splitposition,
                                     int *num_reinsertions) {
  register int i;
  struct Branch b;
  int leftcount = 0, rightcount = 0;

  for (i = 0; i < count; i++) {
    assert(branch[i].child);
    if (branch[i].point.At(splitdimension) < splitposition)
      leftcount++;
    else
      rightcount++;
  }

  int minfill = (int)(MINDATANODEUTIL * NODECARD);
  if (leftcount < minfill)
    if (rightcount < minfill) {
      *num_reinsertions = leftcount + rightcount;
      return BOTH_PARTITIONS_UNDERFULL;
    } else {
      *num_reinsertions = leftcount;
      return LEFT_PARTITION_UNDERFULL;
    }
  else if (rightcount < minfill) {
    *num_reinsertions = rightcount;
    return RIGHT_PARTITION_UNDERFULL;
  } else {
    *num_reinsertions = 0;
    return NEITHER_PARTITION_UNDERFULL;
  }
}

int DataNode::ComputeReinsertions(unsigned int splitdimension,
                                  float splitposition,
                                  ReinsertItem *reinsertion_list,
                                  int *num_points_in_list) {
  int leftcount = 0, rightcount = 0;
  register int i;
  DataNode *replacementNode;
  int target;

  for (i = 0; i < count; i++) {
    assert(branch[i].child);
    if (branch[i].point.At(splitdimension) < splitposition)
      leftcount++;
    else
      rightcount++;
  }

  int minfill = (int)(MINDATANODEUTIL * NODECARD);

  if (leftcount < minfill) {
    if (rightcount < minfill)
      assert(FALSE); // should not happen
    else {
      if (leftcount > 0) {
        replacementNode = new DataNode();
        replacementNode->level = level;
        for (i = 0; i < count; i++) {
          assert(branch[i].child);
          if (branch[i].point.At(splitdimension) < splitposition) {
            reinsertion_list[*num_points_in_list].point = branch[i].point;
            reinsertion_list[*num_points_in_list].tid = (long)branch[i].child;
            (*num_points_in_list)++;
          } else {
            target = replacementNode->Count();
            replacementNode->Branch(target, branch[i]);
            replacementNode->IncrCount();
          }
        }
        CopyFrom(replacementNode);
        delete replacementNode;
      }
      return LEFT_PARTITION_UNDERFULL;
    }
  } else {
    if (rightcount < minfill) {
      if (rightcount > 0) {
        replacementNode = new DataNode();
        replacementNode->level = level;
        for (i = 0; i < count; i++) {
          assert(branch[i].child);
          if (branch[i].point.At(splitdimension) < splitposition) {
            target = replacementNode->Count();
            replacementNode->Branch(target, branch[i]);
            replacementNode->IncrCount();
          } else {
            reinsertion_list[*num_points_in_list].point = branch[i].point;
            reinsertion_list[*num_points_in_list].tid = (long)branch[i].child;
            (*num_points_in_list)++;
          }
        }
        CopyFrom(replacementNode);
        delete replacementNode;
      }
      return RIGHT_PARTITION_UNDERFULL;
    } else
      return NEITHER_PARTITION_UNDERFULL;
  }
}

// all data items right of the partition <splitdimension, splitosition> should
// be transferred from this object to nn
void DataNode::DistributeDataItems(Node *NN, unsigned int splitdimension,
                                   float splitposition) {
  DataNode *replacementNode;
  DataNode *nn = (DataNode *)NN;
  int target;
  register int i;

  assert(nn);
  assert(nn->type == Node::DATANODE);

  replacementNode = new DataNode();
  replacementNode->level = level;
  nn->level = level;

  for (i = 0; i < count; i++) {
    assert(branch[i].child);
    if (branch[i].point.At(splitdimension) < splitposition) {
      target = replacementNode->Count();
      replacementNode->Branch(target, branch[i]);
      replacementNode->IncrCount();
    } else {
      target = nn->Count();
      nn->Branch(target, branch[i]);
      nn->IncrCount();
    }
  }

  CopyFrom(replacementNode);
  delete replacementNode;
}

// if all data items are divided into two groups using the partition
// <splitdimension, splitosition>, would any of the groups be empty
int DataNode::IsEmptyPartition(unsigned int splitdimension, float splitposition,
                               int option) {
  register int i;
  int empty = 1;

  if (option == LEFT) // is the left partition empty??
  {
    for (i = 0; i < count; i++) {
      assert(branch[i].child);
      if (branch[i].point.At(splitdimension) < splitposition) {
        empty = 0; // no, it's not, at least one point in left partition
        return empty;
      }
    }
  } else // is the right partition empty??
  {
    for (i = 0; i < count; i++) {
      assert(branch[i].child);
      if (branch[i].point.At(splitdimension) >= splitposition) {
        empty = 0; // no, it's not, at least one point in right partition
        return empty;
      }
    }
  }
  return empty; // yes, the partition is empty
}

void DataNode::CopyFrom(DataNode *n) {
  int i;
  count = n->count;
  for (i = 0; i < NODECARD; i++)
    branch[i] = n->branch[i];
}

Rect DataNode::CreateELS(unsigned int *ELSarray, Rect *totalspace) {
  int i;
  Rect livespace;
  if (count > 0)
    livespace = Cover();
  else {
    // this is to ensure that livespace is totally contained within totalspace,
    // otherwise problems
    for (i = 0; i < NUMDIMS; i++) {
      livespace.boundary[i] = totalspace->boundary[i];
      livespace.boundary[i + NUMDIMS] = totalspace->boundary[i];
    }
  }
  livespace.EncodeLiveSpace(ELSarray, totalspace);
  return livespace;
}

// compute the radius of the data node given the centroid

float DataNode::Radius(Point *centroid) {
  register int i;
  float distance, radius;

  radius = 0.0;

  for (i = 0; i < count; i++)
    if (branch[i].child) {
      distance = centroid->Distance(&(branch[i].point), DIST_METRIC);
      if (radius < distance)
        radius = distance;
    }
  return radius;
}
