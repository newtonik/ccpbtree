bptree.o: bptree.cc server.h bptree.h
	g++ -Wall -c bptree.cc
all: bptree.o
	g++ -o bpbtree.o
lib: bptree.cc bptree.h
	g++ -fPIC -shared  bptree.cc -o lib.so
contest: lib
	 gcc unittests.c ./lib.so -pthread -o contest
clean:
	rm *.o lib.so bptree
