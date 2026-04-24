#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "MultiPointQuery.h"

MultiPointQuery::MultiPointQuery()
{
 numPoints=0;
}

MultiPointQuery::~MultiPointQuery(){
}

void MultiPointQuery::NormalizeWeights()
{
 int j;
 float sum_of_weights=0.0;
 for(j=0; j<numPoints; j++) sum_of_weights+= weights[j];
 for(j=0; j<numPoints; j++) weights[j]=weights[j]/sum_of_weights;
}

float MultiPointQuery::Distance(Point *P, float (*User_Distance) (Point *, Point *)) 
{
  int j;
  Point *Q;
  float distance=0.0;

  for(j=0; j<numPoints; j++)
  {
    Q=&(querypoints[j]);
    distance += (weights[j])*(User_Distance(P, Q)); // invoke user-specified distance function
  }
return distance;
}

float MultiPointQuery::Distance(Point *P, int dist_func)
{
  int j;
  Point *Q;
  float distance=0.0;

  for(j=0; j<numPoints; j++)
  {
    Q=&(querypoints[j]);
    distance += (weights[j])*(P->Distance(Q, dist_func)); // invoke user-specified distance function
  }
return distance;
}
 

float MultiPointQuery::Distance(MultiPointQuery *MPQ, int dist_func)
{
  int i,j;
  Point *P;
  Point *Q;
  float weight;
  float distance=0.0;

  for(i=0; i<numPoints; i++)
  {
    P=&(querypoints[i]);
    weight=weights[i];
    for(j=0; j<MPQ->numPoints; j++)
    {
       Q=&(MPQ->querypoints[j]);
       distance += weight*(MPQ->weights[j])*(P->Distance(Q, dist_func)); // invoke user-specified distance function
    }
  }
return distance;
} 


float MultiPointQuery::Distance(MultiPointQuery *MPQ, float (*User_Distance) (Point *, Point *))
{
  int i,j;
  Point *P;
  Point *Q;
  float weight;
  float distance=0.0;

  for(i=0; i<numPoints; i++)
  {
    P=&(querypoints[i]);
    weight=weights[i];
    for(j=0; j<MPQ->numPoints; j++)
    {
       Q=&(MPQ->querypoints[j]);
       distance += weight*(MPQ->weights[j])*(User_Distance(P, Q)); // invoke user-specified distance function
    }
  }
return distance;
}



float MultiPointQuery::Distance(Rect *R, int dist_func)
{
  int i, j;
  Point surrogate_point; // replaces the rect appropriately
  Point *P;
  float distance=0.0;

  for(j=0; j<numPoints; j++)
  {
    P=&(querypoints[j]);
    for(i=0; i<NUMDIMS; i++)
      {
        if (P->position[i] < R->boundary[i])
              surrogate_point.position[i] = R->boundary[i];
        else if (P->position[i] > R->boundary[i+NUMDIMS])
              surrogate_point.position[i] = R->boundary[i+NUMDIMS];
        else surrogate_point.position[i] = P->position[i];
       }
   distance += (weights[j])*(P->Distance(&surrogate_point, dist_func));  // invoke user-specified distance function
  }
return distance;
}

float MultiPointQuery::Distance(Rect *R, float (*User_Distance) (Point *, Point *))
{
  int i, j;
  Point surrogate_point; // replaces the rect appropriately
  Point *P;
  float distance=0.0;

  for(j=0; j<numPoints; j++)
  {
    P=&(querypoints[j]);
    for(i=0; i<NUMDIMS; i++)
      {
        if (P->position[i] < R->boundary[i])
              surrogate_point.position[i] = R->boundary[i];
        else if (P->position[i] > R->boundary[i+NUMDIMS])
              surrogate_point.position[i] = R->boundary[i+NUMDIMS];
        else surrogate_point.position[i] = P->position[i];
       }
   distance += (weights[j])*(User_Distance(&surrogate_point, P));  // invoke user-specified distance function
  }
return distance;
}

Point MultiPointQuery::Centroid()
{
  int i, j;
  Point centroid; 

  for(i=0; i<NUMDIMS; i++)
  {
    centroid.position[i]=0.0;
    for(j=0; j<numPoints; j++)
      centroid.position[i] += weights[j]*(querypoints[j].position[i]);
  }
  return centroid;
}

MultiPointQuery &MultiPointQuery::operator = (MultiPointQuery& from) {
  int i;
  numPoints = from.numPoints;
  for (i=0; i < numPoints; i++) {
    querypoints[i] = from.querypoints[i];
    weights[i]=from.weights[i];
  }
  return *this; // allow multiple assignments
}

