bptree.o: bptree.cc server.h bptree.h
	g++ -Wall -c bptree.cc
all: bptree.o
	g++ -o bpbtree.o
clean:
	rm *.o lib.so bptree
