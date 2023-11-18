CC = g++
OBJ = rc.o Token.o

.PHONY: rc clean

rc: main.cpp
	${CC} -g -std=c++20 Token.h main.cpp -o rc 

clean:
	rm rcout rcout.s a.out
