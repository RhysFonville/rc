CC = g++
OBJ = rc.o Token.o

.PHONY: clean run_asm test bear

rc: preprocessor.cpp compiler.cpp util.h Token.h main.cpp
	${CC} -Wno-trigraphs -g -xc++ -std=c++23 Token.h main.cpp -o rc 

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
	${CC} -g -xc++ -std=c++23 test_compiler.cpp -o testrc && \
	./testrc

bear:
	touch compiler.cpp
	bear -- make
