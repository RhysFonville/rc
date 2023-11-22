CC = g++
OBJ = rc.o Token.o

.PHONY: clean run_asm test

rc: compiler.cpp Token.h main.cpp
	${CC} -g -std=c++20 Token.h main.cpp -o rc 

clean:
	rm -f rcout.o rcout.s a.out
	rm -f tests/rcout.s

run_asm:
	as rcout.s -o rcout.o && ld rcout.o -e main && ./a.out

test:
	${CC} -g -std=c++20 tests/test_compiler.cpp -o tests/testrc
	tests/testrc

bear:
	touch compiler.cpp
	bear -- make
