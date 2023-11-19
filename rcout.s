.file "code.txt"
.data
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $323445, -8(%rbp)
movl $45392, -12(%rbp)
movw $1233, -14(%rbp)
movb $34, -15(%rbp)
// Cast
movb $50, -15(%rbp)
// Cast
movw $54, -14(%rbp)
movb -15(%rbp), %al
// Cast
addb -15(%rbp), %al
// Cast
movl %ebx, -12(%rbp)
leaq -12(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %al
leave
ret
.size main, .-main
