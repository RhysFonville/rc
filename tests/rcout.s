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
movq -8(%rbp), %rbx
// Equals
movq %rbx, -12(%rbp)
// Cast
movz q $i, %r10
// Equals
movq %r10, -8(%rbp)
// Cast
movl -12(%rbp), %r11d
// Equals
movl %r11d, -14(%rbp)
// Cast
movz l $s, %r12d
// Equals
movl %r12d, -12(%rbp)
// Cast
movz q $s, %r13
// Equals
movq %r13, -8(%rbp)
// Cast
movz q $c, %r14
// Equals
movq %r14, -8(%rbp)
// Cast
movl -12(%rbp), %r15d
// Equals
movl %r15d, -15(%rbp)
movb $0, %al
leave
ret
.size main, .-main
