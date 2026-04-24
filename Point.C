// FILENAME: Point.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the Point class
// implements functions to read a point,
// compute distances between points etc.
// ------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "Point.h"

Point::Point()
{
  register int i;
  for (i=0; i<NUMDIMS; i++)
    position[i] = (PointReal)0;
}

Point::~Point(){
}

void Point::Init()
{ 
  register int i;
  for (i=0; i<NUMDIMS; i++)
    position[i] = (PointReal)0;
}

int Point::Read(FILE *fp, int *id)
{
  int i;
  if(fp == NULL) return -2;

  
  if (fscanf(fp, "%d", id)<1)
        {
          if(feof(fp)) return -1;
          perror("file read error");
          return -3;
        }
  

  for(i=0; i<NUMDIMS; i++)
    {
      if (fscanf(fp, "%f", &position[i])<1)
	{
	  if(feof(fp)) return -1;   // failure
	  perror("file read error");
	  return -3;                 // failure
	}
    }
  return 0;  // success
}

int Point::ReadNoId(FILE *fp, int *id)
{
  int i;
  if(fp == NULL) return -2;
  *id=0;
  /*
  if (fscanf(fp, "%d", id)<1)
        {
          if(feof(fp)) return -1;
          perror("file read error");
          return -3;
        }
  */

  for(i=0; i<NUMDIMS; i++)
    {
      if (fscanf(fp, "%f", &position[i])<1)
        {
          if(feof(fp)) return -1;   // failure
          perror("file read error");
          return -3;                 // failure
        }
    }
  return 0;  // success
}



PointReal Point::At(int i)
{
  return position[i];
}

void Point::At(int i, PointReal value)
{
  position[i]=value;
}


Rect Point::ToRect()
{
  Rect R;
  register int i;
  for(i=0; i<NUMDIMS; i++)
    {
      R.Low(i, position[i]);
      R.High(i, position[i]);
    }
  return R;
}


void Point::Print()
{
  register int i;

  printf("point:\n");
  for (i = 0; i < NUMDIMS; i++) {
    printf("%f\n", position[i]);
  }
}


float Point::Distance(Point *P, int dist_func)
{
  register Point *p = P;
  register float diff;
  register float distance;
  register float running=0.0;
  register int i;

  switch(dist_func)
    {
    case(L1):
      for(i=0; i<NUMDIMS; i++)
        running+=fabs(position[i]-p->position[i]);
      return running;
      break;

    case (L2):
      for(i=0; i<NUMDIMS; i++)
        {
          diff=position[i]-p->position[i];
          running+=diff*diff;
        }
      distance=sqrt(running);
      return distance;
      break;

    case (LMAX):
      for(i=0; i<NUMDIMS; i++)
        {
          diff=fabs(position[i]-p->position[i]);
          if (running < diff) running=diff;
        }
      return running;
    }
}


float Point::Distance(Point *P, float (*User_Distance) (Point *, Point *))
{
return User_Distance(this, P); // invoke user-specified distance function
}



int Point::Overlap(Point *P, float max_distance, int dist_func)
{
  register Point *p = P; 
  
  if (Distance(p, dist_func) > max_distance) return FALSE;
  else return TRUE;
}

int Point::Overlap(Point *P, float max_distance, float (*User_Distance) (Point *, Point *))
{
  register Point *p = P;

  if (User_Distance(this, p) > max_distance) return FALSE;
  else return TRUE;
}

