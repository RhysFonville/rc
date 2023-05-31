CC = g++
OBJ = rc.o Token.o

asm:
	nasm -felf64 rcout.asm && ld rcout.o && ./a.out