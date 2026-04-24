// FILENAME: decl.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// this file contains all the constants like dimensionality,
// pagesize etc.
// ------------------------------------------------------------


#ifndef DECL_H
#define DECL_H

#define PGSIZE  4096
#define NUMDIMS  16          /* number of dimensions */
#define NDEBUG
#define MINOFNODEUTIL 0.1
#define MINMONODEUTIL 0.4
#define MINDATANODEUTIL 0.3
#define LEFT 0
#define RIGHT 1
#define LEAF 10
#define INTERNAL 20
#define MIN(a,b) ( (a) < (b) ? (a) : (b))
#define MAX(a,b) ( (a) > (b) ? (a) : (b))
#define ELSWANTED 1
#define CASCADING_SPLITS_ALLOWED 1
#define USE_CENTROID 0 // use the centroid, assumed dist_func was known a priori
#define EPSILON 0.2
#define INVALID -1
#define NODE 100
#define OBJECT 200
#define INFTY 2000000       // TWO MILLION , change this when we have larger dbsize
#define MAX_ANSWERS 5000
#define MAX_OBJECTS_SEEN 5000
#define NUM_PAGES_IN_MEMORY 0
#define MAX_PQSIZE 5000

#define MAX_REINSERTIONS_ALLOWED 200
#define NEITHER_PARTITION_UNDERFULL 1
#define RIGHT_PARTITION_UNDERFULL 2
#define LEFT_PARTITION_UNDERFULL 3
#define BOTH_PARTITIONS_UNDERFULL 4
#define SHOULD_NOT_SPLIT 100
#define SHOULD_SPLIT 200


/* distance functions */
#define DIST_METRIC L1
#define RANGE_FACTOR 0.5
#define L1 1
#define MANHATTAN 1
#define L2 2
#define EUCLIDEAN 2
#define L3 3
#define LMAX 100

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* ELS RELATED DEFINITION */

#define ELSPRECISION 4         // no of bits to be used to encode each side
#define NUMGRIDS (((int)pow(2, ELSPRECISION))-1) // number of possible grids in each dim
#define ELSPERINT (((sizeof(unsigned int)*8))/ELSPRECISION) // no of sides encoded in an int
// the 2 defs of ELSARRAYLEN are equivalent; either can be used
// #define ELSARRAYLEN ((int)ceil(((float)(2*NUMDIMS))/ELSPERINT))         // an int array of this size is needed
#define ELSARRAYLEN ((2*NUMDIMS+ELSPERINT-1)/ELSPERINT)        // an int array of this size is needed
#define ELSLEVELS 1

#define NUMSIDES 2*NUMDIMS
#define MAXINSERTS 1200000


#define DATANODEMINFILL (int)(NODECARD*MINDATANODEUTIL)
#define OFNODEMINFILL (int)(OFNODECARD*MINOFNODEUTIL)
#define MONODEMINFILL (int)(MONODECARD*MINMONODEUTIL)

// #define REINSERTLISTSIZE (2*NODECARD)
#define REINSERTLISTSIZE 400

typedef float RectReal;
typedef float PointReal;

struct split
{
        unsigned short splitDim;  /* dimension of split
                                     assumption is #dims < 256 */
        float splitPos;           /* position of split */
        float splitPos2;          /* used for MONode splits */
};

struct ELS
{
  unsigned int elsvalid;
  unsigned int elsarray[ELSARRAYLEN];
};

#endif
