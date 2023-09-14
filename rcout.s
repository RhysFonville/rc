.file "code.txt"
.data
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $46, %rbx
movq $2, %r10
addq %r10, %rbx
movl %rbx, -4(%rbp)
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
