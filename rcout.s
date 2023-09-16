.file "code.txt"
.data
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movb $48, -1(%rbp)
leaq -1(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %ah
leave
ret
.size main, .-main
