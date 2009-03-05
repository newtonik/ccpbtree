bptree.o: bptree.cc server.h bptree.h
	g++ -c bptree.cc
all: bptree.o
	g++ -o bpbtree.o
