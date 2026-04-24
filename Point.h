// FILENAME: Point.h
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Header for Point.C
// defines a multidimensional point
// ------------------------------------------------------------


#ifndef POINT_H
#define POINT_H

#include "decl.h"
#include "Rect.h"

class Rect;

class Point {
 public:
  PointReal position[NUMDIMS];

  Point();
  ~Point();
  void Init();
  int Read(FILE *, int *);
  int ReadNoId(FILE *, int *);
  PointReal At(int);
  void At(int, PointReal);
  void Print();
  float Distance(Point *P, int dist_func);
  float Distance(Point *P,  float (*User_Distance) (Point *, Point *));
  int Overlap(Point *, float max_distance, int dist_func);  // checking overlap with an existing dist func (L1, L2, Lmax)
  int Overlap(Point *, float max_distance, float (*User_Distance) (Point *, Point *)); // checking overlap with an user specified dist func
  Rect ToRect();
};
#endif
