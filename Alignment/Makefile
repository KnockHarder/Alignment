objects = Utility.o seqTrans.o align.o alignMain.o

align : $(objects)
	g++ $(objects) -o align

Utility.o : 

seqTrans.o : Utility.h

align.o : Utility.h

alignMain.o : seqTrans.h align.h

.PHNOY:clean

clean : 
	rm align $(objects)
