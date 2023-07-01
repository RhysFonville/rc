.file "code.txt"
.data
.globl num
.align 1
.type num, @object
num:
.byte 4
.globl s
.align 8
.type s, @object
s:
.asciz "Hello, World!\n"
.text
.globl do_math
.type do_math, @function
do_math:
movq (num), %rax
movq $2, %rdx
mulq %rdx
movq %rax, (num)
movq (num), %rbx
movq $48, %rcx
addq %rcx, %rbx
movq %rbx, (num)
movq $1, %rax
movq $1, %rdi
movq $num, %rsi
movq $1, %rdx
syscall
ret
.globl main
.type main, @function
main:
movq $1, %rax
movq $1, %rdi
movq $s, %rsi
movq $14, %rdx
syscall
call do_math
movq $60, %rax
movq $0, %rdi
syscall
