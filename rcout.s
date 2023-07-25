.file "code.txt"
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
leave
movq $60, %rax
movq $0, %rdi
syscall
.size main, .-main
