CC = g++
OBJ = rc.o Token.o

.PHONY: clean run_asm test bear

rc: compiler.cpp Token.h main.cpp
	${CC} -g -std=c++20 Token.h main.cpp -o rc 

clean:
	rm -f rcout.o rcout.s a.out
	rm -f tests/rcout.s

run_asm:
	as rcout.s -o rcout.o && ld rcout.o -e main && ./a.out

test:
	cd tests && \
	${CC} -g -std=c++20 test_compiler.cpp -o testrc && \
	./testrc

bear:
	touch compiler.cpp
	bear -- make
