// FILENAME: HTreeTest.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// an application that tests the hybrid tree
// ------------------------------------------------------------


#include<stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>


#include "Point.h"
#include "Rect.h"
#include "Node.h"
#include "DataNode.h"
#include "OFNode.h"
#include "MONode.h"
#include "decl.h"
#include "HTree.h"
#include "Query.h"
#include <float.h>
#include <math.h>


FILE *fp;
HTree *h;

// globals
float range;
Point curr_query;

// this one implements L1
float User_Distance(Point *p1, Point *p2)
{
 register int i;
 register float running=0.0;
 register float distance;
 for(i=0; i<NUMDIMS; i++)
   running+=(p1->position[i]-p2->position[i])*(p1->position[i]-p2->position[i]);
   // running+=fabs(p1->position[i]-p2->position[i]);
 distance=sqrt(running);
 // return running;
 return distance;
}

int User_Overlap(Rect *rect, float * dist, int type_of_node)
{
  float distance;
  distance=rect->Distance(&curr_query, L2);
  *dist=distance;
  return 1;
  // if (distance <= range) return 1;
  // else return 0;
}

int SearchTest(char *fn)
{
  int id,i, height;
  int flag=0;
  float beta=0.0, estimated_beta=0.0;
  float avg_beta=0.0, avg_estimated_beta=0.0;
  
  Point point;
  Point querypoint;
  long searchCount, count;
  long *diskacc;
  long *totaldiskacc;
  float *avgdiskacc;
  struct timeval start_time, end_time;
  clock_t query_total_time;
  long diff;
  double time_per_query;
  double time_first_iter=0.0, avg_time_first_iter;
  Result res;
  float alpha;
  Query Q;
  PQ queue;

  fp=fopen(fn, "r");
  if (fp==NULL)
    {
      printf("cannot open file \n");
      exit(1);
    }
  
  height=1+h->height; // to allocate the diskacc structure, 1 more to accomodate the object level
  searchCount=0;
  diskacc=(long *)malloc(height*sizeof(long));
  totaldiskacc=(long *)malloc(height*sizeof(long));
  avgdiskacc=(float *)malloc(height*sizeof(float));
  res.objectList=(struct ReturnedObj *)malloc(MAX_ANSWERS*sizeof(struct ReturnedObj));
 
  // allocate space to hold the query 
  // always single point queries
  Q.startQuery.querypoints=(Point *)malloc(1*sizeof(Point));
  Q.startQuery.weights=(float *)malloc(1*sizeof(float));

  for(i=0; i<height; i++) totaldiskacc[i]=0;
  
  while(querypoint.Read(fp, &id) == 0) 
    {
      // this array will be populated by the search function 
      for(i=0; i<height; i++) diskacc[i]=0;
      // make sure that pq is empty
      while(!(queue.empty())) queue.pop();

      /*
      // start filling up the query fields
      Q.type=Query::NNQUERY;  // or can be range query
      Q.feedback=0;    // not a feedback, posing a fresh query
      Q.getmore=0;
      Q.dist_metric=L2;  // using L2 distance metric
      // Q.dist_func=(float (*) (Point*, Point*))User_Distance;  // not reqd since user function is not used
      Q.use_function=0; // don't use the function provided by the user, use the metric
      Q.k=100;
      Q.range=0.2;  // not required since this is not a range query
      Q.max_returned=MAX_ANSWERS;  // not required
      Q.diskacc=diskacc;
      Q.iteration=0;  // not required
      Q.startQuery.numPoints=1;
      Q.startQuery.querypoints[0]=querypoint;
      Q.startQuery.weights[0]=1.0;
      Q.startQueue=&queue;
      Q.startResult=&res;
      Q.startResult->numObjects=0;
      // rest not needed
      gettimeofday(&start_time, NULL);  
      count=h->Scan(&Q);
      gettimeofday(&end_time, NULL);
      time_per_query=(float)(end_time.tv_sec - start_time.tv_sec)+((float)(end_time.tv_usec-start_time.tv_usec))/1000000;
      time_first_iter += time_per_query;
      */

      searchCount ++;

      
      curr_query=querypoint;
      range=0.2;
      // count=h->Search(User_Overlap, diskacc, &res, MAX_ANSWERS);
      count=h->GetkNext(&queue, User_Overlap, diskacc, 100, &res);

      assert(diskacc[0]==count);
      printf("Query %d: %d \n", searchCount, count);


      /* 
      // print the answers out
      for(int kk=0; kk< res.numObjects; kk++) 
	{
	  printf("%d, ", res.objectList[kk].id, res.objectList[kk].distance);
	  // for(i=0; i<NUMDIMS; i++) printf(" %f", res.objectList[kk].point.position[i]);
	  // printf("\n");
	}
      */

      for(i=0; i<height; i++)
	{
	  totaldiskacc[i]+=diskacc[i];
	  // printf("DA level %d = %d :", i, diskacc[i]);
	}
      printf("\n");
    }
  
  diff = end_time.tv_sec - start_time.tv_sec;
  float total_da_per_query=0.0;
  for(i=0; i<height; i++)
    {
      avgdiskacc[i]=((float)totaldiskacc[i])/searchCount;
      total_da_per_query += avgdiskacc[i];
      printf("Average disk access at level %d = %f \n", i, avgdiskacc[i]);
    }
  total_da_per_query -= avgdiskacc[0];
  printf("Average disk access (total) = %f \n", total_da_per_query);
  printf("Average time taken for 1st iter : %f seconds\n", time_first_iter/searchCount);

  // printf("Average time taken for a search (clock) : %f seconds\n", (float)(query_total_time)/(CLOCKS_PER_SEC*searchCount));
  // printf("Average time taken for a search : %f seconds\n", (float)(diff)/searchCount);
  free(diskacc);
  free(totaldiskacc);
  free(avgdiskacc);
  free(res.objectList);
  free(Q.startQuery.querypoints);
  free(Q.startQuery.weights);
  fclose(fp);
  
  return flag;
}


int main(int argc, char **argv)
{

  char loadFile[40];
  char dataFile[40];
  int resp;
  int flag1=0, flag2=0;
  struct timeval start_time, end_time;
  long diff;

  printf ("Size of a branch is %d **********\n", sizeof(struct Branch));
  printf ("Fanout of normal index nodes is %d ***************\n", NODECARD);

  // printf ("Size of a kdtree internal node %d **********\n", kdTreeInternalNodeSize);
  // printf ("Size of a kdtree leaf node %d **********\n", kdTreeLeafNodeSize);
  printf ("Fanout of overlap free index nodes is %d ***************\n", OFNODECARD);
  printf ("Fanout of overlap minimal index nodes is %d ***************\n", MONODECARD);
  printf("sizeof(struct ELS) = %d\n", sizeof(struct ELS));
  printf ("Size of a kdtree leaf node with els %d **********\n", kdTreeLeafNodewithELSSize);

  
  printf("Load an existing H-tree to memory? (y/n)");
  resp=getchar();
  if (resp == 'y')
    {
      printf("Enter H-tree file:");
      scanf("%s", loadFile);
      gettimeofday(&start_time, NULL);
      printf("Loading......\n");
      h=new HTree(loadFile);
      gettimeofday(&end_time, NULL);
      diff = end_time.tv_sec - start_time.tv_sec;
      printf("Took approx. %d seconds\n", diff);
    }
  else
    {
      printf("Enter data file:");
      scanf("%s", dataFile);
      gettimeofday(&start_time, NULL);
      h=new HTree(dataFile, MAXINSERTS);
      gettimeofday(&end_time, NULL);
      diff = end_time.tv_sec - start_time.tv_sec;
      printf("Took approx. %d seconds\n", diff);
      strcpy(loadFile, "HTdump");
      flag1=1;
    }

  if (!flag1) flag2=SearchTest(argv[1]);

  gettimeofday(&start_time, NULL);
  if (flag1 || flag2) h->HTreeDump(loadFile);
  gettimeofday(&end_time, NULL);
  diff = end_time.tv_sec - start_time.tv_sec;
  printf("Took approx. %d seconds\n", diff);
  
  return(0);
}

