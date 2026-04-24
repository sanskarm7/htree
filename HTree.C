// FILENAME: HTree.C
// ------------------------------------------------------------
// Copyright 1998, The University of California at Irvine
// ------------------------------------------------------------
// Written by Kaushik Chakrabarti
// Implementation of the the hybrid tree.
// implements functions to insert a point into the hybrid tree, 
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


HTree::HTree()
{
  Node *root = new DataNode() ; // the first node is the root as well as leaf (i.e. a data node)
  root->level=0;
  HTreeRoot = root;
  height=1;
  firstitem=1;
  elsvalid=0;
}

HTree::~HTree()
{
}

// ******************************************************************
// ********************* BUILD/LOAD/DUMP CODE  **********************
// ******************************************************************

// bilkload
// each line of the file must contain one record, NUMDIMS float pt numbers separated by spaces
HTree::HTree(char *datafile, int max_inserts) 
{
  FILE *fp;
  int id;
  Point p;
  int count=0;

  // initialize root, height etc.
  Node *root = new DataNode() ; // the first node is the root as well as leaf (i.e. a data node)
  root->level=0;
  HTreeRoot = root;
  height=1;
  firstitem=1;
  elsvalid=0;

  // start processing data file
  fp=fopen(datafile, "r");
  if (fp==NULL)
   {
    printf("cannot open data file \n");
    exit(1);
   }

  while(p.Read(fp, &id) == 0) {

   if (count >= max_inserts) break;
   if ((count % 1000)==0) printf("inserted %d points \n", count);
   Insert(&p, id);
   count ++;
  // for eamonn's data which has no id
  /*
  while(p.ReadNoId(fp, &id) == 0) {
    if (id<=0) id=count+1;
    if (count >= max_inserts) break;
    if ((count % 1000)==0) printf("inserted %d points \n", count);
    Insert(&p, id);
    count ++;
   */
  // end of eamonn's data code
  }
 fclose(fp);
 printf("total inserts = %d \n", count);
}


// load the htree from disk
HTree::HTree(char *dumpfile)
{
  FILE *fp;
  Node *root;
  unsigned long objectcount;
  int loadcount, i, j;
  fp= fopen(dumpfile, "r");
  fread(&rootRect, sizeof(Rect), 1, fp);
  fread(&height, sizeof(int), 1, fp);
  if (height == 1) root = new DataNode();
  else if (height == 2) root = new OFNode(); 
  else root = new MONode(); 
  // the first node is root as well as leaf (i.e. a data node)
  root->level=0;
  HTreeRoot = root;
  loadcount=Load(root, fp);
  firstitem=0;    // since data is loaded from a file, a new insert won't be the first item 
  fclose(fp);
  numemptynodes=0;

  // for(i=0; i< NUMDIMS; i++) splitfreq[i]=0;
  
  // printf ("Some stats.... \n");
  // for(j=0; j< NUMDIMS; j++)
  //   printf("Dimension %d used to split %d times \n", j, splitfreq[j]);

  objectcount=0;
  if (ELSWANTED>0) HTreeCreateELSs(HTreeRoot, &rootRect, &objectcount); 
  printf("Number of empty nodes = %d \n", numemptynodes); 
  printf("Loaded %d nodes from disk \n", loadcount);
  printf("Loaded %d objects from disk \n", objectcount);
}

void HTree::ELS()
{
 unsigned long objectcount;
   if (ELSWANTED>0) HTreeCreateELSs(HTreeRoot, &rootRect, &objectcount);
}


// load the htree from disk
int HTree::Load(Node *n, FILE *fp)
{
  Node *newNode;
  register int nodeCount = 0;
  register int i;
  
  fread(&(n->type), sizeof(int), 1, fp);
  fread(&(n->level), sizeof(int), 1, fp);
  if (n->level > 0) // this is an internal node in the tree 
    {
      if (n->type == Node::MONODE)
	{
	  MONode *N=(MONode *)n;
	  fread(&(N->numInternalNodes), sizeof(int), 1, fp);
	  fread(&(N->numLeafNodes), sizeof(int), 1, fp);

	  for(i=0; i< N->numInternalNodes; i++)
	    {
	      fread(&(N->internal[i]), sizeof(struct OverlappingkdTreeInternalNode), 1, fp);
              // splitfreq[N->internal[i].splitDim]++;
	    }

	  for(i=0; i< N->numLeafNodes; i++)
	    {
	      fread(&(N->leaf[i].parent), sizeof(unsigned int), 1, fp);
	    }
      
	  if (n->level > 2)
	    {
	      for(i=0; i< N->numLeafNodes; i++)
		{
		  N->leaf[i].child = new MONode(); 
		  nodeCount = nodeCount + Load(N->leaf[i].child, fp) + 1;
		} 
	    }
	  else
	    {
	      assert (n->level == 2);
	      for(i=0; i< N->numLeafNodes; i++)
		{
		  N->leaf[i].child = new OFNode(); 
		  nodeCount = nodeCount + Load(N->leaf[i].child, fp) + 1;
		} 
	    }
	  return nodeCount;
	}


      else   // n->type == OVERLAP_FREE
	{
          assert(n->type == Node::OFNODE);
	  OFNode *N=(OFNode *)n;
	  fread(&(N->numInternalNodes), sizeof(int), 1, fp);
	  fread(&(N->numLeafNodes), sizeof(int), 1, fp);

	  for(i=0; i< N->numInternalNodes; i++)
	    {
	      fread(&(N->internal[i]), sizeof(struct kdTreeInternalNode), 1, fp);
              // splitfreq[N->internal[i].splitDim]++;
	    }

	  for(i=0; i< N->numLeafNodes; i++)
	    {
	      fread(&(N->leaf[i].parent), sizeof(unsigned int), 1, fp);
	    }
      
	  for(i=0; i< N->numLeafNodes; i++)
	    {
	      N->leaf[i].child = new DataNode(); 
	      nodeCount = nodeCount + Load(N->leaf[i].child, fp) + 1;
	    } 
	  return nodeCount;
	}
    }
  else // this is a leaf node 
    {
      DataNode *N=(DataNode *)n;
      fread(&(N->count), sizeof(int), 1, fp);

      for(i=0; i< N->count; i++)
	fread(N->branch[i].point.position, (NUMDIMS*sizeof(PointReal)), 1, fp);
      
      for(i=0; i<N->count; i++)
	fread(&(N->branch[i].child), sizeof(Node*), 1, fp);
      return 0;
    }
} 


void HTree::HTreeDump(char *dumpfile)
{
  int dumpcount;
  FILE *fp;
  fp= fopen(dumpfile, "w");
  fwrite(&rootRect, sizeof(Rect), 1, fp);
  fwrite(&height, sizeof(int), 1, fp);
  dumpcount=Dump(HTreeRoot, fp);
  fclose(fp);
  printf("dumped %d nodes on disk \n", dumpcount);
}

int HTree::Dump(Node *n, FILE *fp)
{
  register int nodeCount = 0;
  register int i,j;
  
  if (n->level > 0) // this is an internal node in the tree 
    {
      if (n->type == Node::MONODE)
	{
	  MONode *N=(MONode *)n;
	  fwrite(&(N->type), sizeof(int), 1, fp);
	  fwrite(&(N->level), sizeof(int), 1, fp);
	  fwrite(&(N->numInternalNodes), sizeof(int), 1, fp);
	  fwrite(&(N->numLeafNodes), sizeof(int), 1, fp);
          // printf("type=%d \n", N->type);
          // printf("level=%d \n", N->level);
          // printf("numInternalNodes=%d \n", N->numInternalNodes);
          // printf("numLeafNodes=%d \n", N->numLeafNodes);
          

	  for(i=0; i< N->numInternalNodes; i++)
	    {
	      fwrite(&(N->internal[i]), sizeof(struct OverlappingkdTreeInternalNode), 1, fp);
	    }

	  for(i=0; i< N->numLeafNodes; i++)
	    {
	      fwrite(&(N->leaf[i].parent), sizeof(unsigned int), 1, fp);
	    }
      
	  for(i=0; i< N->numLeafNodes; i++)
	    nodeCount = nodeCount + Dump(N->leaf[i].child, fp) + 1;
      
	  delete N; 
	  return nodeCount;
	}
      else     // n->type == OVERLAP_FREE
	{
          assert(n->type == Node::OFNODE);
	  OFNode *N=(OFNode *)n;
	  fwrite(&(N->type), sizeof(int), 1, fp);
	  fwrite(&(N->level), sizeof(int), 1, fp);
	  fwrite(&(N->numInternalNodes), sizeof(int), 1, fp);
	  fwrite(&(N->numLeafNodes), sizeof(int), 1, fp);
          // printf("type=%d \n", N->type);
          // printf("level=%d \n", N->level);
          // printf("numInternalNodes=%d \n", N->numInternalNodes);
          // printf("numLeafNodes=%d \n", N->numLeafNodes);
          
	  for(i=0; i< N->numInternalNodes; i++)
	    {
	      fwrite(&(N->internal[i]), sizeof(struct kdTreeInternalNode), 1, fp);
	    }

	  for(i=0; i< N->numLeafNodes; i++)
	    {
	      fwrite(&(N->leaf[i].parent), sizeof(unsigned int), 1, fp);
	    }
      
	  for(i=0; i< N->numLeafNodes; i++)
	    nodeCount = nodeCount + Dump(N->leaf[i].child, fp) + 1;
      
	  delete N; 
	  return nodeCount;

	}
	  
    }
  else // this is a leaf node 
    {
      DataNode *N=(DataNode *)n;
      fwrite(&(N->type), sizeof(int), 1, fp);
      fwrite(&(N->level), sizeof(int), 1, fp);
      fwrite(&(N->count), sizeof(int), 1, fp);
      // printf("type=%d \n", N->type);
      // printf("level=%d \n", N->level);
      // printf("count=%d \n", N->count);
      
      for(i=0; i< N->count; i++)
	{
	  fwrite(N->branch[i].point.position, sizeof(PointReal), NUMDIMS, fp);
          // printf("%f %f\n", N->branch[i].point.position[0],  N->branch[i].point.position[NUMDIMS-1]);
	}
      
      for(i=0; i<N->count; i++)
	    fwrite(&(N->branch[i].child), sizeof(Node*), 1, fp);
      delete N; 
      return 0;
    }
} 



// ******************************************************************
// ******************* INSERT CODE BEGINS HERE **********************
// ******************************************************************


 
// Inserts a new data rectangle into the index structure.
// Recursively descends tree, propagates splits back up.
// Returns 0 if node was not split.  Old node updated.
// If node was split, returns 1 and sets the pointer pointed to by
// new_node to point to the new node.  Old node updated to become one of two.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
//

int HTree::HTreeInsert2(Point *r, int tid, Node *n, Node **new_node, struct split **newsplit, int level, Rect *parentNodeRect, int cascading_splits_allowed, ReinsertItem *reinsertList, int *numItems)
{
	register int i;
	struct Branch b;
	Node *n2;
	struct split *lastsplit;
        Rect childNodeRect;
        Rect coverRect;
        int ret_val;

	assert(r);
        assert(n);
        assert(new_node);
	assert(level >= 0 && level <= n->level);

	// Still above level for insertion, go down tree recursively
	//
	if (n->level > level)
	{
	  if (n->type == Node::MONODE)  // minimum overlap node
	    {
	      MONode *N=(MONode *)n;
	      i = N->PickChild(r, parentNodeRect, &childNodeRect);
	      assert(i>=0 && i < N->numLeafNodes);
	      assert(N->leaf[i].child);
	      if (!HTreeInsert2(r, tid, N->leaf[i].child, &n2, &lastsplit, level, &childNodeRect, cascading_splits_allowed, reinsertList, numItems))
		{
		  // data node was not split
		  // nothing needs to be done
		  // something like UpdatePartitions may be needed, check later
		  return 0;
		}
	      else    // child node, the ith leaf node in the node n, was split 
		      // due to the insertion and n2 is the new node
		{
		  ret_val=N->AddChild(n2, (MONode **)new_node, i, lastsplit, newsplit, parentNodeRect);
		  if (*new_node != NULL) assert((((MONode *)*new_node)->numLeafNodes < OFNODECARD) &&  (((MONode *)*new_node)->numLeafNodes > 0));
                  free(lastsplit);
                  return ret_val; 
		}	
	      
	    }

          else               //  n->type == OVERLAP_FREE
	    {
	      OFNode *N=(OFNode *)n;
	      i = N->PickChild(r, parentNodeRect, &childNodeRect);
	      assert(i>=0 && i < N->numLeafNodes);
	      assert(N->leaf[i].child);
	      // invalidate the els
	      N->leaf[i].elsvalid=0;
	      if (!HTreeInsert2(r, tid, N->leaf[i].child, &n2, &lastsplit, level, NULL, cascading_splits_allowed, reinsertList, numItems))
		{
		  // data node was not split
		  // nothing needs to be done
		  // if i was following an eager policy, i would have recomputed the new els here
		  return 0;
		}
	      else    // data node was split and n2 is the new data node
		{
		  ret_val=N->AddChild(n2, (OFNode **) new_node, i, lastsplit, newsplit, parentNodeRect, cascading_splits_allowed, reinsertList, numItems);
		  if (*new_node != NULL) assert((((OFNode *)*new_node)->numLeafNodes < OFNODECARD) &&  (((OFNode *)*new_node)->numLeafNodes > 0));
                  free(lastsplit);
                  return ret_val;
		}	
	    }
	}
	
	// Have reached level for insertion. Add rect, split if necessary
	//
	else if (n->level == level)
	{
	  DataNode *N=(DataNode *)n;
	  b.point = *r;
	  b.child = (Node *) tid;
	  /* child field of leaves contains tid of data record */
	  // printf("adding data item to data node \n");
	  return N->AddChild(&b, (DataNode **) new_node, newsplit);
	}
	else
	  {
	    /* Not supposed to happen */
	    assert (FALSE);
	    return 0;
	  }
}

int HTree::Insert(Point *R, int Tid)
{
       if (firstitem) 
	 {
	   rootRect=R->ToRect();
	   firstitem=0;
	 }
       else rootRect= rootRect.CombineRect(R);

       // allocate space for reinsertions
       ReinsertItem reinsertion_list[MAX_REINSERTIONS_ALLOWED];
       int num_reinsertions=0;
       
       HTreeInsert(R, Tid, &HTreeRoot, 0, 1, reinsertion_list, &num_reinsertions);
       // reinsert the points 
       if (num_reinsertions > 0)
	 {
#ifdef HTREE_DEBUG
	   printf("Numpoints to be reinserted = %d\n", num_reinsertions);
#endif HTREE_DEBUG
	   
	   // for reinsertios, we do not allow cascading splits 
	   // (i.e.  mo type split for the of-nodes) so that there are no
	   // further reinsertions... otherwise this reinsertions will go on
	   // forever
	   for (int i=0; i<num_reinsertions; i++)
	     HTreeInsert(&(reinsertion_list[i].point), reinsertion_list[i].tid, &HTreeRoot, 0); 
	 }
       
}


// Insert a data rectangle into an index structure.
// HTreeInsertRect provides for splitting the root;
// returns 1 if root was split, 0 if it was not.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
// HTreeInsertRect2 does the recursion.
//
int HTree::HTreeInsert(Point *R, int Tid, Node **Root, int Level, int cascading_splits_allowed, ReinsertItem *reinsertList, int *numItems)
{
	register Point *r = R;
	register int tid = Tid;
	register Node **root = Root;
	register int level = Level;
	register int i;
	Node *newnode;
	struct Branch b;
        struct split *newSplit;
	int result;

	assert(r && root);
	assert(level >= 0 && level <= (*root)->level);

	/* for (i=0; i<NUMDIMS; i++)
		assert(r->boundary[i] <= r->boundary[NUMDIMS+i]); */
        assert(*root);
	

	if (HTreeInsert2(r, tid, *root, &newnode, &newSplit, level, &rootRect, cascading_splits_allowed, reinsertList, numItems))  /* root split */
	{
	  if ((*root)->level == 0)   // current root is a data node but new root would be a OFNode
	    {
	      assert((*root)->type == (Node::DATANODE));
	      OFNode *newroot = new OFNode();  /*grow a new root, & tree taller*/

	      newroot->level = (*root)->level + 1;
              newroot->CreatekdTree(*root, newnode, newSplit);
	      *root = newroot;
	      result = 1;
	      height++;
	    }

	  else if ((*root)->level == 1) // current root is a OFNode but new root would be an ordinary node
	    {
	      assert((*root)->type == Node::OFNODE);
	      MONode *newroot= new MONode();
	      newroot->level = (*root)->level + 1;
	      newroot->CreatekdTree(*root, newnode, newSplit);
	      *root = newroot;
	      result = 1;
	      height++;
            }
	  else  // current root is ordinary node, new root is also ordinary root
                // identical to R_tree case
	    {
	      assert((*root)->type == Node::MONODE);
	      MONode *newroot= new MONode(); /* grow a new root, & tree taller */
	      newroot->level = (*root)->level + 1;
	      newroot->CreatekdTree(*root, newnode, newSplit);
	      *root = newroot;
	      result = 1;
	      height++;
	    }
        free(newSplit);
	}
	else
	  result = 0;

	return result;
}

// ******************************************************************
// ******************* INSERT CODE ENDS HERE ************************
// ******************************************************************


// create all the els's
// this is an optimization of the hybrid tree

Rect HTree::HTreeCreateELSs(Node *n, Rect *indexedRegion, unsigned long *objectcount)
{
	register int i, ii;
	Rect livespace;
	Rect newlivespace;
	Rect totallivespace;
	Rect childRectlist[OFNODECARD];
	Rect MOchildRectlist[MONODECARD];
        int first, firsttime;
	Node *childNode;
	Point centroid;
        unsigned long totalcount;
	assert(n);
	assert(n->level >= 0);

        *objectcount=0;
	if (n->level > 0) // this is an internal node in the tree 
	{
	  if (n->type== Node::MONODE)   // this is an MO node
	    {
	      MONode *N= (MONode *)n;
	      N->FillChildList(0, MOchildRectlist, indexedRegion);
	      for(i=0; i<N->numLeafNodes; i++)
		{
		  childNode = (N->leaf[i]).child;
		  livespace=HTreeCreateELSs(childNode, &MOchildRectlist[i], &totalcount);
                  N->leaf[i].count=totalcount;
                  *objectcount +=totalcount;

		  if (i==0) totallivespace=livespace;
		  else totallivespace=totallivespace.CombineRect(&livespace);
		}
	      return totallivespace;
	    }
	  else // this is an OFnode (type is OVERLAP_FREE)
	    {
	      assert(n->type== Node::OFNODE);
	      OFNode *N= (OFNode *)n;
	      N->FillChildList(0, childRectlist, indexedRegion);
              firsttime=1;
	      for(i=0; i<N->numLeafNodes; i++)
		{
		  childNode = ((N->leaf[i]).child);
                  assert(childNode->type == Node::DATANODE);
                  DataNode *dNode=(DataNode *)childNode;
		  // N->leaf[i].ELS=(unsigned int *)malloc(ELSARRAYLEN*sizeof(unsigned int));
		  
		  dNode->CreateELS(N->leaf[i].elsarray, &childRectlist[i]);
		  livespace.DecodeLiveSpace(N->leaf[i].elsarray, &childRectlist[i]);
		  N->leaf[i].elsvalid=1; 
		  // compute centroid
		  
		  centroid=livespace.Center();
		  N->leaf[i].radius = dNode->Radius(&centroid);
                  N->leaf[i].count= dNode->count;
                  *objectcount +=N->leaf[i].count;
		  
                  if (dNode->count == 0) numemptynodes++;
		  
		  if (firsttime) 
		    {
		      totallivespace=livespace;
		      firsttime=0;
                      }
		  else totallivespace=totallivespace.CombineRect(&livespace);
		}
	      return totallivespace;
	    }
	}
}



// this function simulates buffer with lru replacement policy
// pages_in_memory, a dequeue, maintains the list of pages curr in memory
// does eviction based on age

int HTree::IsPageFault(Node *node)
{
  // check whether the page is there in the buffer
  NODELIST::iterator ptr;
  // we are using the memory addresses of the nodes to identify them, as their id
  // this will not work when the nodes are disk-based; but then we won't need a buffer simulator
  if (NUM_PAGES_IN_MEMORY <= 0) return 1;
  assert(pages_in_memory.size() <= NUM_PAGES_IN_MEMORY);
  ptr = find(pages_in_memory.begin(), pages_in_memory.end(), (long)node);
  if (ptr != pages_in_memory.end())  // page is there in memory
    {
      pages_in_memory.erase(ptr);  // remove the page
      pages_in_memory.push_back((long)node); // insert it at the end
      return 0;  // not a page fault
    }
  else   // page is not there in memory
    {
      if (pages_in_memory.size() < NUM_PAGES_IN_MEMORY)  // can just bring it in memory, no one needs to be thrown out
        {
          // nanosleep(&io_delay, NULL);   // do the disk access
          pages_in_memory.push_back((long)node); // bring in the page
        }
      else   // some one has to be thrown out, so the oldest guy is chosen (first guy in list)
        {
          pages_in_memory.pop_front();  // throw the oldest guy out
          // nanosleep(&io_delay, NULL);   // do the disk access
          pages_in_memory.push_back((long)node); // bring in the page
        }
      return 1;
    }
}

void HTree::ClearBuffer()
{
  while(!(pages_in_memory.empty())) pages_in_memory.pop_front();
}

