objects = Utility.o firIndex.o secIndex.o indexMain.o

genIndex : $(objects)
	g++ $(objects) -g -o genIndex 

Utility.o :

firIndex.o : Utility.h

secIndex.o : Utility.h

indexMain.o : firIndex.h secIndex.h

.PHNOY : clean

clean :
	rm genIndex $(objects)
