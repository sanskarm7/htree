// FILENAME: Rect.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the Rect class
// implements functions to read a rect,
// compute distances between rect and point etc.
// ------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "Rect.h"

#define BIG_NUM (FLT_MAX/4.0)
#define Undefined (boundary[0] > boundary[NUMDIMS])


Rect::Rect()
{
  register int i;
  for (i=0; i<NUMSIDES; i++)
    boundary[i] = (RectReal)0;
}

Rect::~Rect(){
}

void Rect::Init()
{
  register int i;
  for (i=0; i<NUMSIDES; i++)
    boundary[i] = (RectReal)0;
}


int Rect::Read(FILE *fp, int *id)
{
  int i;
  if(fp == NULL) return -2;

  if (fscanf(fp, "%d", id)<1)
        {
          if(feof(fp)) return -1;
          perror("file read error");
          return -3;
        }

  for(i=0; i<NUMSIDES; i++)
    {
      if (fscanf(fp, "%f", &boundary[i])<1)
	{
	  if(feof(fp)) return -1;   // failure
	  perror("file read error");
	  return -3;                 // failure
	}
    }
  return 0;  // success
}




RectReal Rect::Low(int i)
{
  return boundary[i];
}

RectReal Rect::High(int i)
{
  return boundary[NUMDIMS+i];
}

void Rect::Low(int i, RectReal value)
{
  boundary[i]=value;
}

void Rect::High(int i, RectReal value)
{
  boundary[NUMDIMS+i]=value;
}


Point Rect::ToPoint()
{
  int i;
  Point p;
  for(i=0; i<NUMDIMS; i++)
    p.At(i, boundary[i]);
  return p;
}


Point Rect::Center()
{
  int i;
  Point p;
  for(i=0; i<NUMDIMS; i++)
    p.At(i, (boundary[i]+boundary[NUMDIMS+i])/2);
  return p;
}

void Rect::Print()
{
  register int i;
  
  printf("rect:\n");
  for (i = 0; i < NUMDIMS; i++) {
    printf("%f\t%f\n",boundary[i], boundary[i + NUMDIMS]);
  }
}


RectReal Rect::Volume()
{
  register int i;
  register RectReal volume = (RectReal)1;
  
  for(i=0; i<NUMDIMS; i++)
    volume *= boundary[i+NUMDIMS] - boundary[i];
  assert(volume >= 0.0);
  return volume;
}


Rect Rect::CombineRect(Rect *Rr)
{
  register Rect *rr = Rr;
  register int i,j;
  Rect new_rect;
  
  for (i = 0; i < NUMDIMS; i++)
    {
      new_rect.Low(i, (MIN(boundary[i], rr->Low(i))));
      j=NUMDIMS+i;
      new_rect.High(i, (MAX(boundary[j], rr->High(i))));
    }
  return new_rect;
}


Rect Rect::CombineRect(Point *r)
{
  register int i, j;
  Rect new_rect;
  
  for (i = 0; i < NUMDIMS; i++)
    {
      new_rect.Low(i, (MIN(boundary[i], r->At(i))));
      j=NUMDIMS+i;
      new_rect.High(i, (MAX(boundary[j], r->At(i))));
    }
  return new_rect;
}


int Rect::Overlap(Rect *S)
{
  register Rect *s = S;
  register int i, j;
  
  for (i=0; i<NUMDIMS; i++)
    {
      j = i + NUMDIMS;  // index for high sides 
      if (boundary[i] > s->High(i) || s->Low(i) > boundary[j])
	return FALSE;
    }
  return TRUE;
}


int Rect::Overlap(Point *S)
{
  register Point *s = S;
  register int i, j;

  for (i=0; i<NUMDIMS; i++)
    {
    j = i + NUMDIMS;  // index for high sides 
      if (boundary[i] > s->At(i) || s->At(i) > boundary[j])
	return FALSE;
    }
  return TRUE;
}


float Rect::Distance(Point *P, int dist_func)
{
  register Point *p = P;
  register float distance;
  register float diff;
  register float running=0.0;
  register int i;

  switch(dist_func)
    {
    case(L1):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i])
            running+=(boundary[i]-p->position[i]);
          else if (p->position[i] > boundary[i+NUMDIMS])
            running+=(p->position[i]-boundary[i+NUMDIMS]);
        }
      return running;
      break;

    case (L2):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i])
            {
              diff=(boundary[i]-p->position[i]);
              running+=diff*diff;
            }
          else if (p->position[i] > boundary[i+NUMDIMS])
            {
              diff=(p->position[i]-boundary[i+NUMDIMS]);
              running+=diff*diff;
            }
        }
      distance=sqrt(running);
      return distance;
      break;

    case (LMAX):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i])
            {
              diff=(boundary[i]-p->position[i]);
              if (running < diff) running=diff;
            }
          else if (p->position[i] > boundary[i+NUMDIMS])
            {
              diff=(p->position[i]-boundary[i+NUMDIMS]);
              if (running<diff) running=diff;
            }
        }
      return running;
    }
}

float Rect::Distance(Point *P, float (*User_Distance) (Point *, Point *))
{
  int i;
  Point surrogate_point; // replaces the rect appropriately

  for(i=0; i<NUMDIMS; i++)
    {
      if (P->position[i] < boundary[i])
            surrogate_point.position[i] = boundary[i];
      else if (P->position[i] > boundary[i+NUMDIMS])
            surrogate_point.position[i] = boundary[i+NUMDIMS];
      else surrogate_point.position[i] = P->position[i];
    }
   return User_Distance(&surrogate_point, P);  // invoke user-specified distance function
   
}


  
int Rect::Overlap(Point *p, float max_distance, int dist_func)
{
  float distance;

  if (Distance(p, dist_func) > max_distance) return FALSE;
  else return TRUE;
}


int Rect::Overlap(Point *P, float max_distance, float (*User_Distance) (Point *, Point *))
{
  int i;
  Point surrogate_point; // replaces the rect appropriately

  for(i=0; i<NUMDIMS; i++)
    {
      if (P->position[i] < boundary[i])
            surrogate_point.position[i] = boundary[i];
      else if (P->position[i] > boundary[i+NUMDIMS])
            surrogate_point.position[i] = boundary[i+NUMDIMS];
      else surrogate_point.position[i] = P->position[i];
    }

  if (User_Distance(&surrogate_point, P) > max_distance) return FALSE;
  else return TRUE;
}

int Rect::Overlap(MultiPointQuery *mpq, float max_distance, int dist_func)
{
  float distance;
  if (mpq->Distance(this, dist_func) > max_distance) return FALSE;
  else return TRUE;
}


int Rect::Overlap(MultiPointQuery *mpq, float max_distance, float (*User_Distance) (Point *, Point *))
{
  float distance;
  if (mpq->Distance(this, User_Distance) > max_distance) return FALSE;
  else return TRUE;

}


float Rect::AmountOfOverlap(Rect *S)
{
  register Rect *s = S;
  register int i, j;
  register float overlap=1.0;
 
  for (i=0; i<NUMDIMS; i++)
    {
      j = i + NUMDIMS;  // index for high sides 
      if (boundary[i] >= s->High(i) || s->Low(i) >= boundary[j])
	return 0;
      else overlap *= MIN(boundary[j], s->High(i)) - MAX(boundary[i], s->Low(i));
    }
  return overlap;
}


int Rect::Contained(Rect *S)
{
  register Rect *s = S;
  register int i, j, result;
  
  result = TRUE;
  for (i = 0; i < NUMDIMS; i++)
   {
     j = i + NUMDIMS;  /* index for high sides */
     result = result
	& boundary[i] >= s->Low(i)
	& boundary[j] <= s->High(i);
    }
  return result;
}



void Rect::EncodeLiveSpace(unsigned int *ELSarray, Rect *totalspace)
{
  int i;
  unsigned int startgrid, endgrid;
  unsigned int shiftedstartgrid, shiftedendgrid;
  double extent, extentof1grid;
  int ELSarrayindex, indexwithinint;
  assert(totalspace);

  for (i=0; i<ELSARRAYLEN; i++) ELSarray[i]=0;
  for(i=0; i<NUMDIMS; i++)
    {
      extent=totalspace->boundary[i+NUMDIMS]-totalspace->boundary[i];
      extentof1grid=extent/NUMGRIDS;
      if (totalspace->boundary[i] >= boundary[i]) startgrid=0;  // shouldn't happen, but may happen due to floating point arithmatic

      else startgrid=(unsigned int)(floor((boundary[i]-totalspace->boundary[i])/extentof1grid));
      if (startgrid < 0) startgrid=0;
      if (startgrid > NUMGRIDS) startgrid=NUMGRIDS;
      if (boundary[i+NUMDIMS] >= totalspace->boundary[i+NUMDIMS]) endgrid=NUMGRIDS; // shouldn't happen, but may happen due to floating point arithmatic

      else endgrid=(unsigned int)(ceil((boundary[i+NUMDIMS]-totalspace->boundary[i])/extentof1grid)
);
      if (endgrid < 0) endgrid=0;
      if (endgrid > NUMGRIDS) endgrid=NUMGRIDS;

      ELSarrayindex=(2*i)/ELSPERINT;
      indexwithinint=(2*i)%ELSPERINT;
      shiftedstartgrid=startgrid << (indexwithinint*ELSPRECISION);
      ELSarray[ELSarrayindex]+=shiftedstartgrid;
      ELSarrayindex=(2*i+1)/ELSPERINT;
      indexwithinint=(2*i+1)%ELSPERINT;
      shiftedendgrid=endgrid<<(indexwithinint*ELSPRECISION);
      ELSarray[ELSarrayindex]+=shiftedendgrid;
    }
}



void Rect::DecodeLiveSpace(unsigned int *ELSarray, Rect *totalspace)
{
  int i;
  unsigned int startgrid, endgrid, value;
  unsigned int shiftedvalue;
  double extent, extentof1grid;
  int ELSarrayindex, indexwithinint;
  int expels=(int)(pow(2,ELSPRECISION));
  int numgrids=expels-1;
  assert(totalspace);

  for(i=0; i<NUMDIMS; i++)
    {
      ELSarrayindex=(2*i)/ELSPERINT;
      value=ELSarray[ELSarrayindex];
      indexwithinint=(2*i)%ELSPERINT;
      shiftedvalue=value>>(indexwithinint*ELSPRECISION);
      startgrid=shiftedvalue%expels;

      /* ELSarrayindex=(2*i+1)/ELSPERINT;  // always the same as before
      value=ELSarray[ELSarrayindex];
      indexwithinint=(2*i+1)%ELSPERINT;
      shiftedvalue=value>>(indexwithinint*ELSPRECISION); */
      shiftedvalue = shiftedvalue >> ELSPRECISION;
      endgrid=shiftedvalue%expels;

      boundary[i]=(totalspace->boundary[i+NUMDIMS]*startgrid + totalspace->boundary[i]*(numgrids-startgrid))/numgrids;

      boundary[i+NUMDIMS]=(totalspace->boundary[i+NUMDIMS]*endgrid + totalspace->boundary[i]*(numgrids-endgrid))/numgrids;
    }
}

/*
float Rect::MaxDist(Point *P, int dist_func)
{
  register Point *p = P;
  register float distance;
  register float diff;
  register float running=0.0;
  register int i;

  switch(dist_func)
    {
    case(L1):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i])
            running+=(boundary[i+NUMDIMS]-p->position[i]);
          else if (p->position[i] > boundary[i+NUMDIMS])
            running+=(p->position[i]-boundary[i]);
          else
            running+=MAX((boundary[i+NUMDIMS]-p->position[i]), (p->position[i]-boundary[i]));
        }
      return running;
      break;

    case (L2):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i]) diff=(boundary[i+NUMDIMS]-p->position[i]);
          else if (p->position[i] > boundary[i+NUMDIMS]) diff=(p->position[i]-boundary[i]);
          else diff=MAX((boundary[i+NUMDIMS]-p->position[i]), (p->position[i]-boundary[i]));
          running+=diff*diff;
        }
      distance=sqrt(running);
      return distance;
      break;

    case (LMAX):
      for(i=0; i<NUMDIMS; i++)
        {
          if (p->position[i] < boundary[i]) diff=(boundary[i+NUMDIMS]-p->position[i]);
          else if (p->position[i] > boundary[i+NUMDIMS]) diff=(p->position[i]-boundary[i]);
          else diff=MAX((boundary[i+NUMDIMS]-p->position[i]), (p->position[i]-boundary[i]));
           if (running<diff) running=diff;
        }
      return running;
    }
}

float Rect::MaxDist(Point *P, float (*User_Distance) (Point *, Point *))
{
  int i;
  Point surrogate_point; // replaces the rect appropriately

  for(i=0; i<NUMDIMS; i++)
    {
      if (P->position[i] < boundary[i])
            surrogate_point.position[i] = boundary[i+NUMDIMS];
      else if (P->position[i] > boundary[i+NUMDIMS])
            surrogate_point.position[i] = boundary[i];
      else if (P->position[i] < ((boundary[i]+boundary[i+NUMDIMS])/2))
            surrogate_point.position[i] = boundary[i+NUMDIMS];
      else surrogate_point.position[i] = boundary[i]; 
    }
   return User_Distance(&surrogate_point, P);  // invoke user-specified distance function

}

*/

