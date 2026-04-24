// FILENAME: HTree.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the the hybrid tree.
// implements functions to search the hybrid tree (range
// and nn-queries, insert a point into the hybrid tree, 
// delete a point from the tree, build the tree from a data
// file provided it is the following format: <id> <float1> ... <floatk>
// (corr. to the k dims), dump an existing htree into a file
// and load the htree back to memory from the dumpfile
// ------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "HTree.h"



int HTree::Scan(Query *query)
{
  int i;
  int how_many, rest;
  PQ *src_queue, *dest_queue, *startQueue;
  Result *src_result;

  if (query->type == Query::RANGEQUERY)  // if it is a range query
    {
      assert(!(query->feedback)); // no question of feedback
      assert(!(query->getmore));  // no question of getmore
      assert(query->startQuery.numPoints > 0 && query->startQuery.querypoints != NULL); // verify startQuery is initialized
      if (query->use_function) // use user specified function
	return Search(&(query->startQuery), query->range, query->dist_func, query->diskacc, query->startResult, query->max_returned);
      else        // use the dist metric specified by the user L1, L2 or LMAX
	return Search(&(query->startQuery), query->range, query->dist_metric, query->diskacc, query->startResult, query->max_returned);
    }
  // END OF RANGE QUERIES
  else    // NN queries
    {
      assert(query->type == Query::NNQUERY); // for now we are handling only NNqueries
      
      // **************** STARTING QUERY ******************

      if(!(query->feedback)) // it is not feedback, it is a fresh query
	{
          query->iteration=0;   // intitialized iteration
	  startQueue=query->startQueue;
	  if (!(query->getmore))
	    {
	      assert(query->startQueue->empty());  // startQueue should be empty
	      assert(query->startResult->numObjects==0); // no results
              // still, make sure it is empty
              query->startResult->numObjects=0;
	      // push in the root
	      startQueue->push(PQEntry(HTreeRoot, NODE, rootRect, 0.0));
	    }
	  else // getting more stuff, continuing from previous query
	    {
	      // make sure startQueue is not empty
	      assert(!(query->startQueue->empty()));
	    }
	  if (query->use_function) // use the user_specified distance function
	    return (GetkNext(startQueue, &(query->startQuery), query->dist_func, query->diskacc, query->k, query->startResult)); // k nearest neighbors 
	  else  // use the distance metric specified by the user
	    return (GetkNext(startQueue, &(query->startQuery), query->dist_metric, query->diskacc, query->k, query->startResult)); // k nearest neighbors
	}

      // **************** REFINED QUERY (FEEDBACK) ******************

      else // feedback
	{
	  printf("This version of Hybrid tree does not support feedback \n");
	  return 0;
	}
    }
}


// ------------------- NN SEARCH FOR START QUERIES ---------------------
// ------------------- NN SEARCH FOR START QUERIES ---------------------
// ------------------- NN SEARCH FOR START QUERIES ---------------------
// NN search for first time searches
// feedback code is above
// for multipoint queries
// Search (plain partial reuse) used this function
// NN query using user specified dist func
// NEAREST NEIGHBOR SEARCH, returns the oid of the NN object

int HTree::GetkNext(PQ *queue, MultiPointQuery *MPQ, float (*User_Distance) (Point *, Point *), long *diskacc, int k, Result *res)
{
	register int i;
	Rect childRectlist[OFNODECARD];
	Rect MOchildRectlist[MONODECARD];
	int sofar=0;

	assert(queue);
	assert(MPQ);


	while(!(queue->empty()))
	  {
	    PQEntry top=queue->top();
	    queue->pop();

	    if (top.type == NODE)
	      {
		Node *node=top.n;
		// if (IsPageFault(node)) diskacc[node->level+1]++;
		diskacc[node->level+1]++;
		if (node->level > 0) // this is an internal node in the tree 
		  {
		    if (node->type== Node::MONODE)   // this is an ordinary node
		      {
			MONode *N=(MONode *)node;
			N->FillChildList(0, MPQ, &(top.BR), queue, User_Distance);
		      }
		    else // this is an OFnode (type is OVERLAP_FREE)
		      {
			assert(node->type== Node::OFNODE);
			OFNode *N=(OFNode *)node;
			N->FillChildList(0, MPQ, &(top.BR), queue, User_Distance);
		      }
		  }
		else // this is a leaf node 
		  {
		    DataNode *N=(DataNode *)node;
		    for (i=0; i<N->count; i++)
		      queue->push(PQEntry((N->branch[i]).child, OBJECT, (N->branch[i].point).ToRect(), MPQ->Distance(&(N->branch[i].point), User_Distance)));
		  }
	      }
	    else
	      {
		diskacc[0]++;
		assert(top.type == OBJECT);
		res->numObjects++;
		(res->objectList[sofar]).id=(long)top.n;
		(res->objectList[sofar]).point=(top.BR).ToPoint();
		(res->objectList[sofar]).distance=top.distance;
		sofar++;
		if (sofar ==k ) // got k nearest neigbors
		return sofar;
	      }
	  }
	return -1;
}


// Search (plain partial reuse) used this function
// NN query using user specified dist func
// NEAREST NEIGHBOR SEARCH, returns the oid of the NN object

int HTree::GetkNext(PQ *queue, MultiPointQuery *MPQ, int dist_func, long *diskacc, int k, Result *res)
{
	register int i;
	Rect childRectlist[OFNODECARD];
	Rect MOchildRectlist[MONODECARD];
	int sofar=0;

	assert(queue);
	assert(MPQ);


	while(!(queue->empty()))
	  {
	    PQEntry top=queue->top();
	    queue->pop();

	    if (top.type == NODE)
	      {
		Node *node=top.n;
		// if (IsPageFault(node)) diskacc[node->level+1]++;
		diskacc[node->level+1]++;
		if (node->level > 0) // this is an internal node in the tree 
		  {
		    if (node->type== Node::MONODE)   // this is an ordinary node
		      {
			MONode *N=(MONode *)node;
			N->FillChildList(0, MPQ, &(top.BR), queue, dist_func);
		      }
		    else // this is an OFnode (type is OVERLAP_FREE)
		      {
			assert(node->type== Node::OFNODE);
			OFNode *N=(OFNode *)node;
			N->FillChildList(0, MPQ, &(top.BR), queue, dist_func);
		      }
		  }
		else // this is a leaf node 
		  {
		    DataNode *N=(DataNode *)node;
		    for (i=0; i<N->count; i++)
		      queue->push(PQEntry((N->branch[i]).child, OBJECT, (N->branch[i].point).ToRect(), MPQ->Distance(&(N->branch[i].point), dist_func)));
		  }
	      }
	    else
	      {
		diskacc[0]++;
		assert(top.type == OBJECT);
		res->numObjects++;
		(res->objectList[sofar]).id=(long)top.n;
		(res->objectList[sofar]).point=(top.BR).ToPoint();
		(res->objectList[sofar]).distance=top.distance;
		sofar++;
		if (sofar ==k ) // got k nearest neigbors
		return sofar;
	      }
	  }
	return -1;
}

