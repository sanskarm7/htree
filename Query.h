#ifndef QUERY_H
#define QUERY_H

#include "Node.h"
#include "decl.h"
#include "Rect.h"
#include "Point.h"
#include "MultiPointQuery.h"
#include <vector>
#include <stack>
#include <queue>      /* for priority_queue */
#include <deque>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

enum OPT_TYPE {FR, SR};  // full reconstruction or selective reconstruction

struct ReturnedObj
{
 long id;
 Point point;
 float distance;
};

struct result
{
 int numObjects;
 struct ReturnedObj *objectList;
};

typedef struct result Result;


struct Object
{
 long id;
 Point point;
};

struct reinsertlist
{
 int numObjects;
 struct Object objectList[REINSERTLISTSIZE];
};

typedef struct reinsertlist ReinsertList;


class PQEntry
{
 public:
  Node *n; // can be node pointer or the object id
  int type; // is it a node or an object depending on which n can be interprted
  Rect BR; // BR when it is a node, feature vector when it is an object
  float distance; // priority is based on this
  float distance_backup;  // backup of the distance, when it is overwritten but may be 
                          // needed again. by backing up, we don't need to recompute
  int count; // number of objects under this node (1 for objects)
  struct ELS els;  // store the ELS of the node
  PQEntry(Node *nodeptr, int t, Rect R, float dist, int cnt=1, unsigned int *els_passed=NULL)
    {
      n=nodeptr;
      type=t;
      BR=R;
      distance=dist;
      count=cnt;
      distance_backup=-1.0;   // initialize to an impossible value
      if (els_passed == NULL) els.elsvalid=FALSE;
      else {
            for(int i=0; i<ELSARRAYLEN; i++) els.elsarray[i]=els_passed[i];
            els.elsvalid=TRUE;
           }
    }

};

class PQCompare
{
 public:
  int operator() (const PQEntry &a, const PQEntry &b)
    {
      return a.distance > b.distance;
    }
};

typedef priority_queue< PQEntry, vector<PQEntry>, PQCompare > PQ;


class Query
{
public:
  enum {NNQUERY, RANGEQUERY};
  int type;         // NN query or range query 
  int feedback;      // flag indicating whether is it feedback or a fresh query :
  int ref_model;     // refinement model, either QUERY_POINT_MOVEMENT
                     // or QUERY_EXPANSION
                     // 1 if feedback, 0 if fresh query
  int getmore;      // flag when set indicates to get more answers to the previous query, 
                    // gets the next bunch of closest matches, the pipeline query processor
                    // will exploit this functionality
  int dist_metric;   // user can either specify a standard dist metric like L1, L2 or LMAX
  float (* dist_func) (Point *, Point *);  // or specify her own dist function
  int use_function;  // if this is set, the user-specified dist func would be used, else
                     // dist_metric would be used 
  int k;             // the number of answers the user wants, for continue queries
                     // the number of additional answers she wants
  float range;       // for range queries, all answers within that range will be returned
  int max_returned;  // for range queries, the user wants at most these many answers
                     // to be returned so that the result buffer is not overwritten 
  long *diskacc;     // statistic returned; the number of disk accesses
                     // to be used to do experiments for writing papers  

  int iteration;     // which iteration of relevance feedback, 0 for the first time
                     // the query is submitted, 1 for the 1st iteration of rel feed,
                                // 2 for second iteration... and so on...
  MultiPointQuery startQuery;   // started with this query
  Point centroid;               // centroid of the points constituting
  PQ *startQueue;               // contains the state for the first result
  Result *startResult;          // start result would returned in this
                                // application must ensure that nough memory is allocated ti contain the results
  
  PQ *tempQueue;                // this is used for temporary purposed by the search algo
  MultiPointQuery tempQuery;

  MultiPointQuery lastQuery;   // query corr. to the latest iteration of relevance feedback
  PQ *lastQueue;               // contains the state for the latest iteration of relevance feedback
  Result *lastResult;       // the result would be returned in this
                            // the program assumes that enough memory is allocated ti contain the results
  OPT_TYPE optimization;         // either FR or SR
  float max_wt_ratio;            // used for SR for qpm

};


typedef deque<long> NODELIST;

struct kdTreeLeafInfo
{
  int leafNodeIndex;
  Rect leafRect;
};


struct numleaves
{
  int numLeavesInSubtree;
  unsigned int leafNode;  // valid iff numLeavesInSubtree==1
  int numLeavesUnderLeftChild;  // valid iff numLeavesInSubtree>1
};

struct lineseg
{
  int leafNodeIndex;
  float leftbound;
  float rightbound;
};




#endif
