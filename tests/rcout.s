.file "tests/test1.txt"
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
movl $9999999999, -19(%rbp)
// Cast
movl -12(%rbp), %eax
cltq
movq %rax,-8(%rbp)
// Cast
movq -8(%rbp), %rbx
movl %ebx,-12(%rbp)
// Cast
movswq -14(%rbp), %rbx
movq %rbx,-8(%rbp)
// Cast
movq -8(%rbp), %rbx
movw %bx,-14(%rbp)
// Cast
movsbq -15(%rbp), %rbx
movq %rbx,-8(%rbp)
// Cast
movq -8(%rbp), %rbx
movb %al,-15(%rbp)
// Cast
movswl -14(%rbp), %ebx
movl %ebx,-12(%rbp)
// Cast
movl -12(%rbp), %ebx
movw %bx,-14(%rbp)
// Cast
movsbl -15(%rbp), %ebx
movl %ebx,-12(%rbp)
// Cast
movl -12(%rbp), %ebx
movb %al,-15(%rbp)
// Cast
movsbw -15(%rbp), %bx
movw %bx,-14(%rbp)
// Cast
movw -14(%rbp), %bx
movb %al,-15(%rbp)
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %al
leave
ret
.size main, .-main
