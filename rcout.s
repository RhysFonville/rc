.file "code.txt"
.data
.text
//// TEST #1 - VARIABLES
//#main
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
//lng l 323445
movq $323445, -8(%rbp)
//int i 45392
movl $45392, -12(%rbp)
//sht s 1233
movw $1233, -14(%rbp)
//ch  c 34
movb $34, -15(%rbp)
//int i_overflow 9999999999
movl $9999999999, -19(%rbp)
//l = i
// Cast
movl -12(%rbp), %eax
cltq
// Equals
movq %rax,-8(%rbp)
//i = l
// Cast
movq -8(%rbp), %rbx
// Equals
movq %rbx,-12(%rbp)
//c = l
// Cast
movq -8(%rbp), %rbx
// Equals
movq %rbx,-15(%rbp)
//l = c
// Cast
movzbq -15(%rbp), %rbx
// Equals
movq %rbx,-8(%rbp)
//#> 0
movb $0, %al
leave
ret
.size main, .-main
