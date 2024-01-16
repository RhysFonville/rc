.file "tests/test3/main.txt"
.data
.globl i
.align 8
.type i, @object
.size i, 4
i:
.word 48
.text
.globl is_true
.type is_true, @function
is_true:
pushq %rbp
movq %rsp, %rbp
movl i(%rip), %ebx
addl $2, %ebx
movl %ebx, i(%rip)
movl i(%rip), %eax
leave
ret
.size is_true, .-is_true
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
movl i(%rip), %ebx
movw %bx, -2(%rbp)
movl i(%rip), %ebx
movl %ebx, -6(%rbp)
movb $2, %bl
cmpw %bx, -2(%rbp)
jne .IF0
movl $0, %eax
call is_true
movl %eax, -6(%rbp)
.IF0:
leaq -6(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movl i(%rip), %ebx
cmpw %bx, -2(%rbp)
jne .IF1
movl $0, %eax
call is_true
movl %eax, -6(%rbp)
.IF1:
leaq -6(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movl $10, -6(%rbp)
leaq -6(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %al
leave
ret
.size main, .-main
