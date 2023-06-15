num:
.word 5
.text
.globl _start
_start:
movq $1, %rax
movq $1, %rdi
movq $5, %r9 # initial value for r9
addq $5, %r9 # addition
addq $48, %r9 # turn to ascii number
push %rax # push rax to stack
movq %rsi, %rsp # set rsi to stack pointer
movq $2, %rdx
syscall
movq $60, %rax
movq $0, %rdi
syscall
