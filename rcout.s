.file "code.txt"
.data
.globl s
.align 0
.type s, @object
s:
.asciz "hello\n"
.text
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
leaq s, %rbx
movq $1, %rax
movq $1, %rdi
movq %rbx, %rsi
movq $6, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %ah
leave
ret
.size main, .-main
