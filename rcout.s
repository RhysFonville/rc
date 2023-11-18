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
movq %rax,-8(%rbp)
//i = l
// Cast
movq -8(%rbp), %rbx
movl %ebx,-12(%rbp)
//l = s
// Cast
movswq -14(%rbp), %rbx
movq %rbx,-8(%rbp)
//s = l
// Cast
movq -8(%rbp), %rbx
movw %bx,-14(%rbp)
//l = c
// Cast
movsbq -15(%rbp), %rbx
movq %rbx,-8(%rbp)
//c = l
// Cast
movq -8(%rbp), %rbx
movb %al,-15(%rbp)
//i = s
// Cast
movswl -14(%rbp), %ebx
movl %ebx,-12(%rbp)
//s = i
// Cast
movl -12(%rbp), %ebx
movw %bx,-14(%rbp)
//i = c
// Cast
movsbl -15(%rbp), %ebx
movl %ebx,-12(%rbp)
//c = i
// Cast
movl -12(%rbp), %ebx
movb %al,-15(%rbp)
//s = c
// Cast
movsbw -15(%rbp), %bx
movw %bx,-14(%rbp)
//c = s
// Cast
movw -14(%rbp), %bx
movb %al,-15(%rbp)
//>e 0
movq $60, %rax
movq $0, %rdi
syscall
//#> 0
movb $0, %al
leave
ret
.size main, .-main
