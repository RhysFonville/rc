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
movl -12(%rbp), %ebx
movzlq %ebx, -8(%rbp)
movq -8(%rbp), %rbx
movq %rbx, $i
movw -14(%rbp), %ax
movzwl %ax, -12(%rbp)
movl -12(%rbp), %ebx
movl %ebx, $s
movq -8(%rbp), %rbx
movq %rbx, $s
movq -8(%rbp), %rbx
movq %rbx, $c
movb -15(%rbp), %al
movzbl %al, -12(%rbp)
