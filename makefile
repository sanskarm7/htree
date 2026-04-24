
SPLIT_TYPE= q


# .SUFFIXES: .obj .cpp .c .rsp .exp

#CC = cl
CC = g++ 
#CC = quantify g++ 
#CC = purify g++ 
LIB_DIRS        = -L.
LIBS            = -lrtree
#CFLAGS = -c -g -DHTREE_DEBUG
CFLAGS = -c
#CFLAGS = -Ic:\msvc20\include
#EXT = obj
EXT = o
LIB= librtree.a
#LIB= rtree.lib
OBJS= Node.$(EXT) DataNode.$(EXT) OFNode.$(EXT) MONode.$(EXT) HTree.$(EXT) HTreeRangeSearch.$(EXT) HTreeSearch.$(EXT) MultiPointQuery.$(EXT) Point.$(EXT) Rect.$(EXT) HTreeTest.$(EXT)
AR = ar -rv $@
TESTPGM = HTreeTest
#AR = lib /out:rtree.lib $(OBJS)

.C.$(EXT):
	$(CC) $(CFLAGS)  $<


$(TESTPGM): $(OBJS)
	$(CC) -g -o $(TESTPGM) $(OBJS) -lm

clean:
	rm -f *.$(EXT) 

