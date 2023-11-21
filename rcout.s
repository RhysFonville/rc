.file "code.txt"
.data
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movb $10, -1(%rbp)
movl $52, -5(%rbp)
movb $3, %bl
movb 3, %bl
addb -5(%rbp), %bl
movl %ebx, -9(%rbp)
leaq -9(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
leaq -1(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $10, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %al
leave
ret
.size main, .-main
