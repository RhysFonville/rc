CC = g++
OBJ = rc.o Token.o

.PHONY: clean run_asm test bear

rc: compiler.cpp Token.h main.cpp
	${CC} -Wno-trigraphs -g -std=c++20 Token.h main.cpp -o rc 

clean:
	find . -type f -name 'core' -exec rm -f {} \;
	find . -type f -name 'a.out' -exec rm -f {} \;
	find . -type f -name 'rcout.o' -exec rm -f {} \;
	find . -type f -name 'rcout.s' -exec rm -f {} \;
	find . -type f -name 'rc' -exec rm -f {} \;

run_asm:
	as rcout.s -o rcout.o && ld rcout.o -e main && ./a.out

test:
	cd tests && \
	${CC} -g -std=c++20 test_compiler.cpp -o testrc && \
	./testrc

bear:
	touch compiler.cpp
	bear -- make
