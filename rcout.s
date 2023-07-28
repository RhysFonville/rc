.file "code.txt"
.data
.text
.globl babi
.type babi, @function
babi:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movl $42, -4(%rbp)
leaq -4(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movl -4(%rbp), %eax
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
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %ah
leave
ret
.size main, .-main
