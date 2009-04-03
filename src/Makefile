btree.o: btree.cc btree.h
	g++ -Wall -fpermissive -c btree.cc
all: btree.o
	g++ -o btree.o
lib: bptree.cc bptree.h
	g++ -fPIC -shared  bptree.cc -o lib.so
test: btree.h test.cc cscope
	g++ -Wall -fpermissive btree.h test.cc -o test
contest: lib
	 gcc unittests.c ./lib.so -pthread -o contest
cscope: 
	cscope -b -k
clean:
	rm *.o *.out test
