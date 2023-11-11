CC = g++
OBJ = testrc.o Token.o

.PHONY: testrc clean

testrc: test_compiler.cpp
	${CC} -g -std=c++17 ../Token.h test_compiler.cpp -o testrc
