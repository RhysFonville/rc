CC = g++
OBJ = testrc.o Token.o

.PHONY: testrc

testrc: test_compiler.cpp
	${CC} -g -std=c++17 ../Token.h test_compiler.cpp -o testrc
	./testrc
	rm a.out
