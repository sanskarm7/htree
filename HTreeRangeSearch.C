// FILENAME: HTree.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the the hybrid tree.
// implements functions to do range search over the hybrid tree 
// ------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "HTree.h"


// -------------------------- range searches ---------------------------
  
// bounding box queries

int HTree::Search(Rect *R, long *diskacc, Result *res, int max_returned)
{
  res->numObjects=0;
  return (HTreeSearch(HTreeRoot, R, diskacc, res, max_returned));         // bounding box based
}

// distance-based range search with in-built distance function
int HTree::Search(MultiPointQuery *mpq, float range, int dist_func, long *diskacc, Result *res, int max_returned)
{
  res->numObjects=0;
  if (USE_CENTROID) assert(dist_func == DIST_METRIC);
  return (HTreeSearch(HTreeRoot, mpq, range, dist_func, diskacc, &rootRect, res, max_returned)); // distance based
}

// distance-based range search with user-defined distance function
int HTree::Search(MultiPointQuery *mpq, float range, float (*User_Distance) (Point *, Point *), long *diskacc,Result *res, int max_returned )
{
  res->numObjects=0;
  return (HTreeSearch(HTreeRoot, mpq, range, User_Distance, diskacc, &rootRect, res, max_returned)); // distance based
}

// ----------------------------------------------------------------------

// DISTANCE BASED SEARCH


// Search in an index tree or subtree for all data retangles that
// overlap the argument rectangle.
// Returns the number of qualifying data rects.
//
int HTree::HTreeSearch(Node *n, MultiPointQuery *mpq, float distance, int dist_func, long *diskacc, Rect *indexedRegion, Result *res, int max_returned)
{
	register int hitCount = 0;
	register int i;
	// int visitlist[OFNODECARD];
	struct region visitlist[OFNODECARD];
	int visitcount=0;
	// int MOvisitlist[MONODECARD];
	struct region MOvisitlist[MONODECARD];
	int MOvisitcount=0;
        int first;
	float distance_from_query;

	assert(n);
	assert(n->level >= 0);
	assert(mpq);

	diskacc[n->level+1]++;
	if (n->level > 0) // this is an internal node in the tree 
	{
	  if (n->type== Node::MONODE)   // this is an ordinary node
	    {
              MONode *N=(MONode *)n;
	      N->FillVisitList(mpq, 0, MOvisitlist, &MOvisitcount, indexedRegion, distance, dist_func);
	      for(i=0; i<MOvisitcount; i++)
		{
		  // printf("Accessing a node at level %d \n", n->level);
		  hitCount += HTreeSearch((N->leaf[MOvisitlist[i].index]).child, mpq, distance, dist_func, diskacc, &(MOvisitlist[i].indexedRegion), res, max_returned);
		}
	    }
	  else // this is an OFnode (type is OVERLAP_FREE)
	    {
	      assert(n->type== Node::OFNODE);
              OFNode *N=(OFNode *)n;
	      N->FillVisitList(mpq, 0, visitlist, &visitcount, indexedRegion, distance, dist_func);
	      for(i=0; i<visitcount; i++)
		{
		  //printf("Accessing a node at level %d \n", N->level);
		  hitCount += HTreeSearch((N->leaf[visitlist[i].index]).child, mpq, distance, dist_func, diskacc, &(visitlist[i].indexedRegion), res, max_returned);
		}
	    }
	}
	else // this is a leaf node 
	{
                DataNode *N=(DataNode *)n;
		for (i=0; i<N->count; i++)
		  {
		    assert(N->branch[i].child);
		    distance_from_query=mpq->Distance(&(N->branch[i].point),dist_func);
		    if (distance_from_query <= distance)
		      {
			// printf("Accessing a node at level %d \n", n->level);
                        if (res->numObjects < max_returned)
                          {
                            (res->objectList[res->numObjects]).id=(long)N->branch[i].child;
                            (res->objectList[res->numObjects]).point=N->branch[i].point;
                            (res->objectList[res->numObjects]).distance =distance_from_query;
			    res->numObjects++;
			    diskacc[n->level]++;
			    hitCount++;
			  }
		      }
		  }
	}
	return hitCount;
}



// DISTANCE BASED SEARCH


// Search in an index tree or subtree for all data retangles that
// overlap the argument rectangle.
// Returns the number of qualifying data rects.
//
int HTree::HTreeSearch(Node *n, MultiPointQuery *mpq, float distance, float (*User_Distance) (Point *, Point *), long *diskacc, Rect *indexedRegion, Result *res, int max_returned)
{
	register int hitCount = 0;
	register int i;
	// int visitlist[OFNODECARD];
	struct region visitlist[OFNODECARD];
	int visitcount=0;
	// int MOvisitlist[MONODECARD];
	struct region MOvisitlist[MONODECARD];
	int MOvisitcount=0;
        int first;
	float distance_from_query;

	assert(n);
	assert(n->level >= 0);
	assert(mpq);

        diskacc[n->level+1]++;
	if (n->level > 0) // this is an internal node in the tree 
	{
	  if (n->type== Node::MONODE)   // this is an ordinary node
	    {
              MONode *N=(MONode *)n;
	      N->FillVisitList(mpq, 0, MOvisitlist, &MOvisitcount, indexedRegion, distance, User_Distance);
	      for(i=0; i<MOvisitcount; i++)
		{
		  //printf("Accessing a node at level %d \n", n->level);
		  hitCount += HTreeSearch((N->leaf[MOvisitlist[i].index]).child, mpq, distance, User_Distance, diskacc, &(MOvisitlist[i].indexedRegion), res, max_returned);
		}
	    }
	  else // this is an OFnode (type is OVERLAP_FREE)
	    {
	      assert(n->type== Node::OFNODE);
              OFNode *N=(OFNode *)n;
	      N->FillVisitList(mpq, 0, visitlist, &visitcount, indexedRegion, distance, User_Distance);
	      for(i=0; i<visitcount; i++)
		{
		  //printf("Accessing a node at level %d \n", N->level);
		  hitCount += HTreeSearch((N->leaf[visitlist[i].index]).child, mpq, distance, User_Distance, diskacc, &(visitlist[i].indexedRegion), res, max_returned);
		}
	    }
	}
	else // this is a leaf node 
	{
                DataNode *N=(DataNode *)n;
		for (i=0; i<N->count; i++)
		  {
		    assert(N->branch[i].child);
		    distance_from_query=mpq->Distance(&(N->branch[i].point),User_Distance);
		    if (distance_from_query <= distance)
		      {
			// printf("Accessing a node at level %d \n", n->level);
			if (res->numObjects < max_returned) 
			  {
			    
			    (res->objectList[res->numObjects]).id=(long)N->branch[i].child;
			    (res->objectList[res->numObjects]).point=N->branch[i].point;
			    (res->objectList[res->numObjects]).distance =distance_from_query;
			    res->numObjects++;
			    diskacc[n->level]++;
			    hitCount++;
			  }
		      }
		  }
	}
	return hitCount;
}

// BOUNDING BOX BASED SEARCH


int HTree::HTreeSearch(Node *n, Rect *R, long *diskacc, Result *res, int max_returned)
{
	register Rect *r = R;
	register int hitCount = 0;
	register int i;
	int visitlist[OFNODECARD];
	int visitcount=0;
	int MOvisitlist[MONODECARD];
	int MOvisitcount=0;
        int first;

	assert(n);
	assert(n->level >= 0);
	assert(r);

        if (IsPageFault(n)) diskacc[n->level+1]++;
	if (n->level > 0) // this is an internal node in the tree 
	{
	  if (n->type== Node::MONODE)   // this is a monode
	    {
              MONode *N=(MONode *)n;
	      N->FillVisitList(r, 0, MOvisitlist, &MOvisitcount);
	      for(i=0; i<MOvisitcount; i++)
		{
		  //printf("Accessing a node at level %d \n", N->level);
		  hitCount += HTreeSearch((N->leaf[MOvisitlist[i]]).child, R, diskacc, res, max_returned);
		}
	    }
	  else // this is an OFnode 
	    {
	      assert(n->type== Node::OFNODE);
              OFNode *N=(OFNode *)n;
	      N->FillVisitList(r, 0, visitlist, &visitcount);
	      for(i=0; i<visitcount; i++)
		{
		  //printf("Accessing a node at level %d \n", N->level);
		  hitCount += HTreeSearch((N->leaf[visitlist[i]]).child, R, diskacc, res, max_returned);
		}
	    }
	}
	else // this is a leaf node 
	{
                DataNode *N=(DataNode *)n;
		for (i=0; i<N->count; i++)
		  {
		    assert(N->branch[i].child);
		    if (r->Overlap(&(N->branch[i].point)))
		      {
			// printf("Accessing a node at level %d \n", N->level);
			if (diskacc[n->level] < max_returned) 
			  {
                            (res->objectList[res->numObjects]).id=(long)N->branch[i].child;
                            (res->objectList[res->numObjects]).point=N->branch[i].point;
                            // (res->objectList[res->numObjects]).distance  can't be evaluated 
			    res->numObjects++;
			    diskacc[n->level]++;
			    hitCount++;
			  }
		      }
		  }
	}
	return hitCount;
}
	
