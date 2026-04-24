#ifndef MULTI_POINT_QUERY_H
#define MULTI_POINT_QUERY_H

#include "decl.h"
#include "Point.h"
#include "Rect.h"

class Point;
class Rect;

class MultiPointQuery
{
public:
 int numPoints;
 Point *querypoints;
 float *weights;

 MultiPointQuery();
 ~MultiPointQuery();
 void NormalizeWeights();
// distance from a single point
 float Distance(Point *P,  float (*User_Distance) (Point *, Point *)); 
 float Distance(Point *P,  int dist_func); 
// distance from another multipoint query all pairs distance
 float Distance(MultiPointQuery *MPQ, float (*User_Distance) (Point *, Point *));  
 float Distance(MultiPointQuery *MPQ, int dist_func); 
// distance from a rectange (mindist)
 float Distance(Rect *R, float (*User_Distance) (Point *, Point *));
 float Distance(Rect *R, int dist_func);
 Point Centroid();
 MultiPointQuery &operator = (MultiPointQuery&);  // copy constructor
};

#endif

