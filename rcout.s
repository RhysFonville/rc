.file "code.txt"
.data
.text
.globl print
.type print, @function
print:
push %rbp
mov %rsp, %rbp
subq $4, %rsp
movq $48, -4(%rbp)
leaq -4(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movl $0, %eax
leave
ret
.globl main
.type main, @function
main:
push %rbp
mov %rsp, %rbp
call print
leaq -4(%rbp), %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movq $60, %rax
movq $2, %rdi
syscall
movl $0, %eax
leave
ret
