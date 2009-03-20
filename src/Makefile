btree.o: btree.cc btree.h
	g++ -Wall -fpermissive -c btree.cc
all: btree.o
	g++ -o btree.o
lib: bptree.cc bptree.h
	g++ -fPIC -shared  bptree.cc -o lib.so
contest: lib
	 gcc unittests.c ./lib.so -pthread -o contest
clean:
	rm *.o
