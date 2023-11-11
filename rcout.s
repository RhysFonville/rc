.file "resume.txt"
.data
.text
//# on line 1
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
// Multiple token choices on line 1
// Multiple token choices on line 2
movb $10, -1(%rbp)
// Multiple token choices on line 4
movl $48, -5(%rbp)
// Multiple token choices on line 5
movl $3, -9(%rbp)
// Multiple token choices on line 7
//& on line 7
leaq -5(%rbp), %rbx
//> on line 7
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
// Multiple token choices on line 8
//& on line 8
leaq -1(%rbp), %rbx
//> on line 8
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
// Multiple token choices on line 10
// Multiple token choices on line 10
// Multiple token choices on line 10
// Multiple token choices on line 10
movl -5(%rbp), %ebx
movl -9(%rbp), %r10d
addl %r10d, %ebx
//= on line 10
movl %ebx, -5(%rbp)
// Multiple token choices on line 12
//& on line 12
leaq -5(%rbp), %rbx
//> on line 12
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
// Multiple token choices on line 13
//& on line 13
leaq -1(%rbp), %rbx
//> on line 13
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
//> on line 15
movq $60, %rax
movq $0, %rdi
syscall
//#> on line 16
movb $0, %al
leave
ret
.size main, .-main
