.file "code.txt"
.data
.text
.globl babi
.type babi, @function
babi:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movl $21, -4(%rbp)
movl $2, -8(%rbp)
movb $41, %ah
movb $1, %r10b
addb %r10b, %ah
movl %ah, -12(%rbp)
movl -12(%rbp), %eax
leave
ret
.size babi, .-babi
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movl $0, %eax
call babi
movl %eax, -4(%rbp)
leaq -4(%rbp), %r10
movq $1, %rax
movq $1, %rdi
movq %r10, %rsi
movq $1, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %ah
leave
ret
.size main, .-main
