// FILENAME: Rect.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Header for Rect.C
// defines a multidimensional rectangle
// ------------------------------------------------------------


#ifndef RECT_H
#define RECT_H

#include "decl.h"
#include "Point.h"
#include "MultiPointQuery.h"

class Point;
class MultiPointQuery;

class Rect {

 public:
  RectReal boundary[NUMSIDES]; /* xmin,ymin,...,xmax,ymax,... */

  Rect();
  ~Rect();
  void Init();
  int Read(FILE *, int *);
  RectReal Low(int);
  RectReal High(int);
  void Low(int, RectReal);
  void High(int, RectReal);
  Point ToPoint();
  Point Center();
  void Print();
  RectReal Volume();
  Rect CombineRect(Rect*);
  Rect CombineRect(Point*);
  int Overlap(Rect*);
  int Overlap(Point*);
  float Distance(Point *P, int dist_func);
  float Distance(Point *P,  float (*User_Distance) (Point *, Point *));
  float MaxDist(Point *P, int dist_func);
  float MaxDist(Point *P,  float (*User_Distance) (Point *, Point *));
  int Overlap(Point *, float max_distance, int dist_func);
  int Overlap(Point *, float max_distance, float (*User_Distance) (Point *, Point *));
  int Overlap(MultiPointQuery *mpq, float max_distance, int dist_func);
  int Overlap(MultiPointQuery *mpq, float max_distance, float (*User_Distance) (Point *, Point *));
  float AmountOfOverlap(Rect*);
  int Contained(Rect *S);
  void EncodeLiveSpace(unsigned int *els, Rect *totalspace);
  void DecodeLiveSpace(unsigned int *els, Rect *totalspace);
};

struct region
{
  Rect indexedRegion;       // the BR corresponding to the child
  int index;                // the leaf index, which leaf is this child?
};

#endif
