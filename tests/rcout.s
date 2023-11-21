.file "tests/test2.txt"
.data
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movb $10, -1(%rbp)
movl $5, -5(%rbp)
movb $2, -6(%rbp)
movq $48, -14(%rbp)
movw $8, -16(%rbp)
movq $60, %rax
movb $0, %dh
syscall
movb $0, %al
leave
ret
.size main, .-main
